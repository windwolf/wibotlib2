#include "modbus.hpp"
#include <cstring>

namespace wibot {

// 协议常量
static constexpr u8 kMinRequestSize = 8;  // 最小请求：地址+功能码+数据+CRC

ModbusSlave::ModbusSlave(UartStream& uart, u8 slaveAddr, IModbusSlaveHandler& handler)
    : _uart(uart), _slaveAddr(slaveAddr), _handler(handler), _receivedLength(0) {
}

ModbusSlave::~ModbusSlave() {
}

u16 ModbusSlave::process() {
    // 尝试从串口读取数据
    auto result = _uart.read(_buffer.toSlice(MODBUS_BUFFER_SIZE - _receivedLength, _receivedLength))
                      .wait(0);  // 非阻塞读取

    if (!result.isOk()) {
        return 0;
    }

    // 假设Result有getValue()或类似方法获取读取的字节数
    // 这里需要根据实际的Result实现来调整
    u16 bytesRead = 0;  // TODO: 从result获取实际读取字节数
    if (bytesRead == 0) {
        return 0;
    }

    _receivedLength += bytesRead;

    // 检查是否接收到完整帧（至少需要最小长度）
    if (_receivedLength >= kMinRequestSize) {
        processRequest();
        _receivedLength = 0;  // 重置接收缓冲区
    }

    return bytesRead;
}

bool ModbusSlave::validateRequest(u16 length) {
    // 检查地址是否匹配
    if (_buffer[0] != _slaveAddr && _buffer[0] != 0) {  // 0为广播地址
        return false;
    }

    // 验证CRC
    Slice bufSlice(_buffer.data, length);
    _crc16.reset();
    _crc16.calculate(_buffer.data, length - 2);
    u16 recvCrc = bufSlice.getUint16(length - 2, Endian::kLittle);
    return _crc16.get() == recvCrc;
}

void ModbusSlave::processRequest() {
    if (!validateRequest(_receivedLength)) {
        return;  // CRC错误或地址不匹配，忽略
    }

    u8 functionCode = _buffer[1];

    // 根据功能码调用相应处理函数
    switch (functionCode) {
        case 0x01:  // Read Coils
            handleRead(RegisterType::kCoil, functionCode, true);
            break;
        case 0x02:  // Read Discrete Inputs
            handleRead(RegisterType::kDiscreteInput, functionCode, true);
            break;
        case 0x03:  // Read Holding Registers
            handleRead(RegisterType::kHoldingRegister, functionCode, false);
            break;
        case 0x04:  // Read Input Registers
            handleRead(RegisterType::kInputRegister, functionCode, false);
            break;
        case 0x05:  // Write Single Coil
            handleWrite(RegisterType::kCoil, functionCode, true, true);
            break;
        case 0x06:  // Write Single Register
            handleWrite(RegisterType::kHoldingRegister, functionCode, true, false);
            break;
        case 0x0F:  // Write Multiple Coils
            handleWrite(RegisterType::kCoil, functionCode, false, true);
            break;
        case 0x10:  // Write Multiple Registers
            handleWrite(RegisterType::kHoldingRegister, functionCode, false, false);
            break;
        default:
            sendExceptionResponse(functionCode, ExceptionCode::kIllegalFunction);
            break;
    }
}

void ModbusSlave::handleRead(RegisterType type, u8 functionCode, bool isBitType) {
    // 先从_buffer读取请求参数
    Slice bufSlice(_buffer.data, _receivedLength);
    u16   addr  = bufSlice.getUint16(2, Endian::kBig);
    u16   count = bufSlice.getUint16(4, Endian::kBig);

    // 计算数据字节数：位类型按位计算，寄存器类型每个2字节
    u16 byteCount = isBitType ? (count + 7) / 8 : count * 2;

    // 复用_buffer构造响应（请求已处理完，可以覆盖）
    Slice respData(_buffer.toSlice(byteCount, 3));

    Result result = _handler.onRead(type, addr, count, respData);
    if (!result.isOk()) {
        sendExceptionResponse(functionCode, ExceptionCode::kIllegalDataAddress);
        return;
    }

    // 构造响应：地址 + 功能码 + 字节数 + 数据 + CRC
    _buffer[0] = _slaveAddr;
    _buffer[1] = functionCode;
    _buffer[2] = byteCount;

    u16 respLength = 3 + byteCount;
    _crc16.reset();
    _crc16.calculate(_buffer.data, respLength);
    u16 crc = _crc16.get();
    Slice(_buffer.data, respLength + 2).setUint16(respLength, crc, Endian::kLittle);

    sendResponse(Slice(_buffer.data, respLength + 2));
}

void ModbusSlave::handleWrite(RegisterType type, u8 functionCode, bool isSingle, bool isBitType) {
    Slice bufSlice(_buffer.data, _receivedLength);
    u16   addr = bufSlice.getUint16(2, Endian::kBig);

    if (isSingle) {
        // 单个写入：从offset 4读取值
        u16 value = bufSlice.getUint16(4, Endian::kBig);

        if (isBitType) {
            // Coil: 转换0xFF00为1字节
            u8     coilValue = (value == 0xFF00) ? 1 : 0;
            Slice  valueData(&coilValue, 1);
            Result result = _handler.onWrite(type, addr, 1, valueData);
            if (!result.isOk()) {
                sendExceptionResponse(functionCode, ExceptionCode::kIllegalDataAddress);
                return;
            }
        } else {
            // 寄存器: 转换为大端字节序
            u8 regData[2];
            Slice(regData, 2).setUint16(0, value, Endian::kBig);
            Slice  valueData(regData, 2);
            Result result = _handler.onWrite(type, addr, 1, valueData);
            if (!result.isOk()) {
                sendExceptionResponse(functionCode, ExceptionCode::kIllegalDataAddress);
                return;
            }
        }

        // 单个写入响应：回显请求
        sendResponse(Slice(_buffer.data, 8));
    } else {
        // 多个写入：从offset 4读取数量，offset 6读取字节数，offset 7开始是数据
        u16   count     = bufSlice.getUint16(4, Endian::kBig);
        u8    byteCount = _buffer[6];
        Slice data(&_buffer.data[7], byteCount);

        Result result = _handler.onWrite(type, addr, count, data);
        if (!result.isOk()) {
            sendExceptionResponse(functionCode, ExceptionCode::kIllegalDataAddress);
            return;
        }

        // 多个写入响应：地址 + 功能码 + 起始地址 + 数量 + CRC
        // 复用_buffer构造响应
        _buffer[0] = _slaveAddr;
        _buffer[1] = functionCode;
        Slice(_buffer.data, 8).setUint16(2, addr, Endian::kBig);
        Slice(_buffer.data, 8).setUint16(4, count, Endian::kBig);

        _crc16.reset();
        _crc16.calculate(_buffer.data, 6);
        u16 crc = _crc16.get();
        Slice(_buffer.data, 8).setUint16(6, crc, Endian::kLittle);

        sendResponse(Slice(_buffer.data, 8));
    }
}

void ModbusSlave::sendExceptionResponse(u8 functionCode, ExceptionCode exCode) {
    // 复用_buffer构造异常响应
    _buffer[0] = _slaveAddr;
    _buffer[1] = functionCode | 0x80;  // 功能码最高位置1表示异常
    _buffer[2] = static_cast<u8>(exCode);

    _crc16.reset();
    _crc16.calculate(_buffer.data, 3);
    u16 crc = _crc16.get();
    Slice(_buffer.data, 5).setUint16(3, crc, Endian::kLittle);

    sendResponse(Slice(_buffer.data, 5));
}

void ModbusSlave::sendResponse(const Slice& response) {
    _uart.write(response).wait(TIMEOUT_FOREVER);
}

}  // namespace wibot

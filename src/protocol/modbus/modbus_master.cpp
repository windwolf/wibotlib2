#include "modbus.hpp"
#include <cstring>

namespace wibot::modbus {

// 协议常量
static constexpr u8 kModbusResponseHeaderSize = 3;  // 设备地址 + 功能码 + 字节数
static constexpr u8 kModbusCrcSize            = 2;
static constexpr u8 kModbusSimpleResponseSize = 8;  // 单寄存器写入响应大小
static constexpr u8 kSimpleCmdSize            = 6;  // 不含CRC的简单命令长度

ModbusMaster::~ModbusMaster() {
}

// 获取寄存器类型对应的读功能码
static inline u8 getReadFunctionCode(RegisterType type) {
    switch (type) {
        case RegisterType::kCoil:
            return 0x01;
        case RegisterType::kDiscreteInput:
            return 0x02;
        case RegisterType::kHoldingRegister:
            return 0x03;
        case RegisterType::kInputRegister:
            return 0x04;
        default:
            return 0x00;  // Invalid
    }
}

// 获取寄存器类型对应的写功能码
static inline u8 getWriteFunctionCode(RegisterType type, bool isSingle) {
    switch (type) {
        case RegisterType::kCoil:
            return isSingle ? 0x05 : 0x0F;
        case RegisterType::kHoldingRegister:
            return isSingle ? 0x06 : 0x10;
        default:
            return 0x00;  // Invalid
    }
}

// 判断寄存器类型是否为位类型（Coil或DiscreteInput）
static inline bool isBitRegister(RegisterType type) {
    return type == RegisterType::kCoil || type == RegisterType::kDiscreteInput;
}

// 计算数据字节长度
static inline u16 calcDataLength(RegisterType type, u16 count) {
    return isBitRegister(type) ? (count + 7) / 8 : count * 2;
}

// 通用读取函数
Result ModbusMaster::read(RegisterType type, u8 deviceAddr, u16 addr, u16 length,
                          const Slice& data) {
    u8   functionCode = getReadFunctionCode(type);
    auto ar = sendSimpleCommand(deviceAddr, functionCode, addr, length).wait(TIMEOUT_FOREVER);
    if (!ar.isOk()) {
        return ar;
    }

    // 计算响应长度和数据长度
    u16 dataLength = calcDataLength(type, length);
    u16 respLength = kModbusResponseHeaderSize + dataLength + kModbusCrcSize;

    ar = _uart.read(_buffer.toSlice(respLength)).wait(TIMEOUT_FOREVER);
    if (!ar.isOk()) {
        return ar;
    }

    if (_buffer[0] != deviceAddr || _buffer[1] != functionCode ||
        !validateCrc(_buffer, respLength - kModbusCrcSize)) {
        return Result(Result::ResultStatus::kError, ErrorCode::kInvalidResponse);
    }

    // 复制数据
    memcpy(data.data, &_buffer.data[kModbusResponseHeaderSize], dataLength);
    return Result::kOk;
}

// 通用写入函数（带响应）
Result ModbusMaster::write(RegisterType type, u8 deviceAddr, u16 addr, u16 length,
                           const Slice& data) {
    // 只有Coil和HoldingRegister支持写入
    if (type != RegisterType::kCoil && type != RegisterType::kHoldingRegister) {
        return Result(Result::ResultStatus::kError, ErrorCode::kInvalidResponse);
    }

    u8 functionCode = getWriteFunctionCode(type, length == 1);

    // 单个寄存器写入（0x05或0x06）
    Result result;
    if (length == 1) {
        u16 value = data.getUint16(0, Endian::kBig);
        result    = sendSimpleCommand(deviceAddr, functionCode, addr, value).wait(TIMEOUT_FOREVER);
        if (!result.isOk()) {
            return result;
        }
    } else {
        // 多个寄存器写入（0x0F或0x10）
        result = sendMultiWriteCommand(type, deviceAddr, functionCode, addr, length, data)
                     .wait(TIMEOUT_FOREVER);
        if (!result.isOk()) {
            return result;
        }
    }

    // 读取并验证响应（8字节固定格式）
    result = _uart.read(_buffer.toSlice(kModbusSimpleResponseSize)).wait(TIMEOUT_FOREVER);
    if (!result.isOk()) {
        return result;
    }

    if (_buffer[0] != deviceAddr || _buffer[1] != functionCode ||
        !validateCrc(_buffer, kModbusSimpleResponseSize - kModbusCrcSize)) {
        return Result(Result::ResultStatus::kError, ErrorCode::kInvalidResponse);
    }

    return Result::kOk;
}

// 通用写入函数（无响应）
AsyncResult ModbusMaster::writeWithoutResponse(RegisterType type, u8 deviceAddr, u16 addr,
                                               u16 length, const Slice& data) {
    if (type != RegisterType::kCoil && type != RegisterType::kHoldingRegister) {
        return AsyncResult::fromResult(
            Result(Result::ResultStatus::kError, ErrorCode::kInvalidResponse));
    }

    u8 functionCode = getWriteFunctionCode(type, length == 1);

    if (length == 1) {
        u16 value = data.getUint16(0, Endian::kBig);
        return sendSimpleCommand(deviceAddr, functionCode, addr, value);
    }

    // 多个寄存器写入（0x0F或0x10）
    return sendMultiWriteCommand(type, deviceAddr, functionCode, addr, length, data);
}

// 发送多寄存器写入命令的公共方法
AsyncResult ModbusMaster::sendMultiWriteCommand(RegisterType type, u8 deviceAddr, u8 functionCode,
                                                u16 addr, u16 length, const Slice& data) {
    u16 byteCount = calcDataLength(type, length);
    u16 cmdLength = 7 + byteCount + kModbusCrcSize;

    // 检查缓冲区是否足够
    if (cmdLength > MODBUS_BUFFER_SIZE) {
        return AsyncResult::fromResult(
            Result(Result::ResultStatus::kError, ErrorCode::kInvalidResponse));
    }

    // 使用成员变量 _buffer，避免栈分配
    Slice cmdSlice(_buffer.data, cmdLength);

    cmdSlice.setUint8(0, deviceAddr);
    cmdSlice.setUint8(1, functionCode);
    cmdSlice.setUint16(2, addr, Endian::kBig);
    cmdSlice.setUint16(4, length, Endian::kBig);
    cmdSlice.setUint8(6, byteCount);

    memcpy(&_buffer.data[7], data.data, byteCount);

    _crc16.reset();
    _crc16.calculate(_buffer.data, 7 + byteCount);
    u16 crc = _crc16.get();
    cmdSlice.setUint16(7 + byteCount, crc, Endian::kLittle);

    return _uart.write(cmdSlice);
}

AsyncResult ModbusMaster::sendSimpleCommand(u8 deviceAddr, u8 functionCode, u16 regAddr,
                                            u16 lengthOrValue) {
    // 使用成员变量 _buffer，避免栈分配
    Slice buf(_buffer.data, kModbusSimpleResponseSize);
    buf.setUint8(0, deviceAddr);
    buf.setUint8(1, functionCode);
    buf.setUint16(2, regAddr, Endian::kBig);
    buf.setUint16(4, lengthOrValue, Endian::kBig);
    _crc16.reset();
    _crc16.calculate(_buffer.data, kSimpleCmdSize);
    u16 crc = _crc16.get();
    buf.setUint16(kSimpleCmdSize, crc, Endian::kLittle);
    return _uart.write(buf);
};

bool ModbusMaster::validateCrc(const Slice& buf, u16 length) {
    _crc16.reset();
    _crc16.calculate(buf.data, length);
    u16 recvCrc = buf.getUint16(length, Endian::kLittle);
    return _crc16.get() == recvCrc;
};

}  // namespace wibot::modbus

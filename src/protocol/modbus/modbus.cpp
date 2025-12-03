#include "modbus.hpp"
#include <cstring>

namespace wibot {

ModbusMaster::~ModbusMaster() {
}

AsyncResult ModbusMaster::sendSimpleCommand(u8 deviceAddr, u8 functionCode, u16 regAddr,
                                            u16 lengthOrValue) {
    Buffer<8> cmd;
    Slice     buf(cmd);
    buf.setUint8(0, deviceAddr);
    buf.setUint8(1, functionCode);
    buf.setUint16(2, regAddr, Endian::kBig);
    buf.setUint16(4, lengthOrValue, Endian::kBig);
    _crc16.reset();
    _crc16.calculate(buf.data, 6);
    u16 crc = _crc16.get();
    buf.setUint16(6, crc);
    return _uart.write(cmd);
};

Result ModbusMaster::readCoils(u8 deviceAddr, u16 addr, u16 length, const Slice& data) {
    auto ar = sendSimpleCommand(deviceAddr, 0x01, addr, length).wait(TIMEOUT_FOREVER);
    if (!ar.isOk()) {
        return ar;
    }
    auto respLength = 6 + length / 8 + 1;
    ar              = _uart.read(_buffer.toSlice(respLength)).wait(TIMEOUT_FOREVER);
    if (!ar.isOk()) {
        return ar;
    }
    if (_buffer[0] != deviceAddr || _buffer[1] != 0x01 || !validateCrc(_buffer, respLength - 2)) {
        return Result(Result::ResultStatus::kError, ErrorCode::kInvalidResponse);
    }
    memcpy(data.data, &_buffer.data[4], length / 8 + 1);
    return Result::kOk;
};

Result ModbusMaster::writeCoils(u8 deviceAddr, u16 addr, u16 length, const Slice& data) {
    return Result::kOk;
};
AsyncResult ModbusMaster::writeCoilsWithoutResponse(u8 deviceAddr, u16 addr, u16 length,
                                                    const Slice& data) {
    return AsyncResult::fromResult(Result::kOk);
};

Result ModbusMaster::readDiscreteInputs(u8 deviceAddr, u16 regAddr, const Slice& data) {
    return Result::kOk;
};

Result ModbusMaster::readHoldingRegisters(u8 deviceAddr, u16 regAddr, u16 length,
                                          const Slice& data) {
    auto ar = sendSimpleCommand(deviceAddr, 0x03, regAddr, length).wait(TIMEOUT_FOREVER);
    if (!ar.isOk()) {
        return ar;
    }
    auto respLength = 6 + length * 2;
    ar              = _uart.read(_buffer.toSlice(respLength)).wait(TIMEOUT_FOREVER);
    if (!ar.isOk()) {
        return ar;
    }
    if (_buffer[0] != deviceAddr || _buffer[1] != 0x03 || !validateCrc(_buffer, respLength - 2)) {
        return Result(Result::ResultStatus::kError, ErrorCode::kInvalidResponse);
    }
    memcpy(data.data, &_buffer.data[4], length * 2);
    return Result::kOk;
};

Result ModbusMaster::writeHoldingRegister(u8 deviceAddr, u16 regAddr, u16 value) {
    auto ar = sendSimpleCommand(deviceAddr, 0x06, regAddr, value).wait(TIMEOUT_FOREVER);
    if (!ar.isOk()) {
        return ar;
    }
    auto respLength = 8;
    ar              = _uart.read(_buffer.toSlice(respLength)).wait(TIMEOUT_FOREVER);
    if (!ar.isOk()) {
        return ar;
    }
    if (_buffer[0] != deviceAddr || _buffer[1] != 0x06 || !validateCrc(_buffer, respLength - 2)) {
        return Result(Result::ResultStatus::kError, ErrorCode::kInvalidResponse);
    }
    return Result::kOk;
};

bool ModbusMaster::validateCrc(const Slice& buf, u16 length) {
    _crc16.reset();
    _crc16.calculate(buf.data, length);
    u16 crc = _crc16.get();
    u16 recvCrc = buf.getUint16(length, Endian::kLittle);
    return crc == recvCrc;
};

AsyncResult ModbusMaster::writeHoldingRegisterWithoutResponse(u8 deviceAddr, u16 regAddr,
                                                              u16 value) {
    auto ar = sendSimpleCommand(deviceAddr, 0x06, regAddr, value);
    return ar;
};

Result ModbusMaster::writeHoldingRegisters(u8 deviceAddr, u16 regAddr, u16 length,
                                           const Slice& data) {
    return Result::kOk;
};

Result ModbusMaster::readInputRegisters(u8 deviceAddr, u16 regAddr, const Slice& data) {
    return Result::kOk;
};

}  // namespace wibot

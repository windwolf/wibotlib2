#pragma once

#include "bus.hpp"
#include "crc16.hpp"

#ifndef MODBUS_BUFFER_SIZE
#define MODBUS_BUFFER_SIZE 8
#endif

namespace wibot {

class ModbusMaster {
   public:
    enum ErrorCode : u8 {
        kInvalidResponse = 0x01,
    };

   public:
    ModbusMaster(UartStream& uart) : _uart(uart) {};
    ~ModbusMaster();

   public:
    Result      readCoils(u8 deviceAddr, u16 addr, u16 length, const Slice& data);
    Result      writeCoils(u8 deviceAddr, u16 addr, u16 length, const Slice& data);
    AsyncResult writeCoilsWithoutResponse(u8 deviceAddr, u16 addr, u16 length, const Slice& data);
    Result      readDiscreteInputs(u8 deviceAddr, u16 regAddr, const Slice& data);
    Result      readHoldingRegisters(u8 deviceAddr, u16 regAddr, u16 length, const Slice& data);
    Result      writeHoldingRegister(u8 deviceAddr, u16 regAddr, u16 value);
    Result      writeHoldingRegisters(u8 deviceAddr, u16 regAddr, u16 length, const Slice& data);
    AsyncResult writeHoldingRegisterWithoutResponse(u8 deviceAddr, u16 regAddr, u16 value);
    AsyncResult writeHoldingRegistersWithoutResponse(u8 deviceAddr, u16 regAddr, u16 length,
                                                     const Slice& data);
    Result      readInputRegisters(u8 deviceAddr, u16 regAddr, const Slice& data);

   private:
    AsyncResult sendSimpleCommand(u8 deviceAddr, u8 functionCode, u16 regAddr, u16 lengthOrValue);
    bool        validateCrc(const Slice& buf, u16 length);

   private:
    UartStream&                _uart;
    Buffer<MODBUS_BUFFER_SIZE> _buffer;
    Crc16Validator             _crc16{Crc16Validator::CRC16_MODBUS};
};

}  // namespace wibot

#pragma once

#include "bus.hpp"
#include "crc16.hpp"

#ifndef MODBUS_BUFFER_SIZE
#define MODBUS_BUFFER_SIZE 255  // 支持多寄存器读写的最大缓冲区
#endif

namespace wibot::modbus {

enum class RegisterType : u8 {
    kCoil,
    kDiscreteInput,
    kHoldingRegister,
    kInputRegister,
};

class ModbusMaster {
   public:
    enum ErrorCode : u8 {
        kInvalidResponse = 0x01,
    };

   public:
    ModbusMaster(UartStream& uart) : _uart(uart) {};
    ~ModbusMaster();

   public:
    // 通用读取函数
    Result      read(RegisterType type, u8 deviceAddr, u16 addr, u16 length, const Slice& data);
    // 通用写入函数（带响应）
    Result      write(RegisterType type, u8 deviceAddr, u16 addr, u16 length, const Slice& data);
    // 通用写入函数（无响应）
    AsyncResult writeWithoutResponse(RegisterType type, u8 deviceAddr, u16 addr, u16 length,
                                     const Slice& data);

   private:
    AsyncResult sendSimpleCommand(u8 deviceAddr, u8 functionCode, u16 regAddr, u16 lengthOrValue);
    AsyncResult sendMultiWriteCommand(RegisterType type, u8 deviceAddr, u8 functionCode, u16 addr,
                                      u16 length, const Slice& data);
    bool        validateCrc(const Slice& buf, u16 length);

   private:
    UartStream&                _uart;
    Buffer<MODBUS_BUFFER_SIZE> _buffer;
    Crc16Validator             _crc16{Crc16Validator::Modbus()};
};

// ============================================================================
// ModbusSlave - Modbus从机实现
// ============================================================================

/**
 * @brief Modbus从机请求处理回调接口
 */
class IModbusSlaveHandler {
   public:
    virtual ~IModbusSlaveHandler() = default;

    /**
     * @brief 读取寄存器（通用）
     * @param type 寄存器类型
     * @param addr 起始地址
     * @param count 数量
     * @param data 输出数据缓冲区
     * @return 成功返回kOk，失败返回错误码
     */
    virtual Result onRead(RegisterType type, u16 addr, u16 count, Slice& data) = 0;

    /**
     * @brief 写入寄存器（通用）
     * @param type 寄存器类型
     * @param addr 起始地址
     * @param count 数量（单个寄存器时为1）
     * @param data 输入数据
     * @return 成功返回kOk，失败返回错误码
     */
    virtual Result onWrite(RegisterType type, u16 addr, u16 count, const Slice& data) = 0;
};

/**
 * @brief Modbus从机
 * 
 * 监听串口接收Modbus请求，解析后调用回调处理，并发送响应
 */
class ModbusSlave {
   public:
    enum ExceptionCode : u8 {
        kIllegalFunction         = 0x01,
        kIllegalDataAddress      = 0x02,
        kIllegalDataValue        = 0x03,
        kSlaveDeviceFailure      = 0x04,
        kAcknowledge             = 0x05,
        kSlaveDeviceBusy         = 0x06,
        kMemoryParityError       = 0x08,
        kGatewayPathUnavailable  = 0x0A,
        kGatewayTargetNoResponse = 0x0B,
    };

   public:
    ModbusSlave(UartStream& uart, u8 slaveAddr, IModbusSlaveHandler& handler);
    ~ModbusSlave();

    /**
     * @brief 处理接收到的数据（在主循环或中断中调用）
     * @return 处理的字节数
     */
    u16 process();

    /**
     * @brief 设置从机地址
     */
    void setSlaveAddress(u8 addr) {
        _slaveAddr = addr;
    }

    /**
     * @brief 获取从机地址
     */
    u8 getSlaveAddress() const {
        return _slaveAddr;
    }

   private:
    void processRequest();
    void sendResponse(const Slice& response);
    void sendExceptionResponse(u8 functionCode, ExceptionCode exCode);
    bool validateRequest(u16 length);

    // 通用处理函数
    void handleRead(RegisterType type, u8 functionCode, bool isBitType);
    void handleWrite(RegisterType type, u8 functionCode, bool isSingle, bool isBitType);

   private:
    UartStream&                _uart;
    u8                         _slaveAddr;
    IModbusSlaveHandler&       _handler;
    Buffer<MODBUS_BUFFER_SIZE> _buffer;
    Crc16Validator             _crc16{Crc16Validator::Modbus()};
    u16                        _receivedLength;
};

}  // namespace wibot::modbus

#pragma once

//
// Created by zhouj on 2023/10/6.
//

#include "type.hpp"
#include "os/async.hpp"
#include "circular-buffer.hpp"

namespace wibot {

enum class FeedEvent : u8 {
    kFeedOnIdle = 0x01,
    kFeedOnHalf = 0x02,
    kFeedOnFull = 0x04,
    kFeedOnAll  = kFeedOnIdle | kFeedOnHalf | kFeedOnFull,
};

/**
 * @brief 周期性异步事件源
 * 
 */
class AsyncEventSource {
   public:
    /**
    * @brief 订阅异步事件, 返回可重复触发的异步结果
    * 
    * @return AsyncResult 
    */
    virtual AsyncResult subscribe(FeedEvent feedEvents)  = 0;
    virtual Result      start(CircularBuffer8 &rxBuffer) = 0;
    virtual Result      stop()                           = 0;
};

template <typename T>
class AsyncReader {
   public:
    /**
      * @brief 读取数据到缓冲区, 直到填满data缓冲区为止才发出完成通知.
      * 
      * @param data 
      * @return AsyncResult 
      */
    virtual AsyncResult read(T &data) = 0;
};

template <typename T>
class AsyncWriter {
   public:
    /**
     * @brief 写入数据, 直到完成才发出完成通知.
     * 
     * @note `write`不能并行使用
     * @note 调用者需确保在异步通知到达前, data缓冲区不会被修改或释放.
     * @param data 
     * @return AsyncResult 
     */
    virtual AsyncResult write(const T &data) = 0;
};

template <typename T>
class SyncReader {
   public:
    /**
      * @brief 读取数据到缓冲区, 直到填满data缓冲区为止才发出完成通知.
      * 
      * @param data 
      * @return AsyncResult 
      */
    virtual Result read(T &data, u32 timeout = TIMEOUT_FOREVER) = 0;
};

template <typename T>
class SyncWriter {
   public:
    /**
      * @brief 读取数据到缓冲区, 直到填满data缓冲区为止才发出完成通知.
      * 
      * @param data 
      * @return AsyncResult 
      */
    virtual Result write(const T &data, u32 timeout = TIMEOUT_FOREVER) = 0;
};

template <typename T>
class AsyncRegisterClient {
   public:
    virtual AsyncResult readReg(u16 regAddr, T &data)        = 0;
    virtual AsyncResult writeReg(u16 regAddr, const T &data) = 0;
};

//----------------------------------------------------------------------------

enum UartWordLength : u8 {
    k7B = 1,
    k8B = 0,
    k9B = 2,
};
enum UartStopBits : u8 {
    k0_5 = 0,
    k1   = 1,
    k1_5 = 2,
    k2   = 3,
};
enum UartParity : u8 {
    kNone = 0,
    KEven = 1,
    KOdd  = 2,
};

struct UartConfig {
    u32            baudrate;
    UartWordLength wordLength;
    UartStopBits   stopBits;
    UartParity     parity;
    bool           notIgnoreParityError  : 1;
    bool           notIgnoreFrameError   : 1;
    bool           notIgnoreOverrunError : 1;
    bool           notIgnoreNoiseError   : 1;
};

class UartStream : public AsyncEventSource,
                   public AsyncReader<const Slice>,
                   public AsyncWriter<const Slice> {
   public:
    virtual Result setConfig(UartConfig &config) = 0;
};

enum class SpiCpha : u8 {
    k1Edge = 0,
    k2Edge = 1,
};
enum class SpiCpol : u8 {
    k0 = 0,
    k1 = 1,
};

struct SpiConfig {
    SpiCpha   cpha;
    SpiCpol   cpol;
    // bool      autoCs;
    DataWidth dataWidth;
    u32       baudrate;
};

class SpiMaster : public AsyncReader<const Slice>, public AsyncWriter<const Slice> {
   public:
    virtual Result      setConfig(const SpiConfig &config)                  = 0;
    virtual AsyncResult writeRead(const Slice &txData, const Slice &rxData) = 0;
    virtual Result      begin()                                             = 0;
    virtual Result      end()                                               = 0;
};

struct I2cMasterTransitionConfig {
    u16       deviceAddr;
    bool      is10BitsAddr;
    DataWidth dataWidth;
};

class I2cMaster : public AsyncRegisterClient<const Slice>,
                  public AsyncReader<const Slice>,
                  public AsyncWriter<const Slice> {
   public:
    Result setTransitionConfig(u16 deviceAddr) {
        I2cMasterTransitionConfig config{
            .deviceAddr   = deviceAddr,
            .is10BitsAddr = false,
            .dataWidth    = DataWidth::k8Bits,
        };
        return setTransitionConfig(config);
    };

    virtual Result setTransitionConfig(I2cMasterTransitionConfig &config) = 0;
};

};  // namespace wibot

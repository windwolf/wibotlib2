#pragma once

//
// Created by zhouj on 2023/10/6.
//

#include "type.hpp"
#include "model.hpp"
namespace wibot {

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

class UartStream : public AsyncEventSource, public AsyncReader<Slice>, public AsyncWriter<Slice> {
   public:
    virtual Result setConfig(UartConfig& config) = 0;
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
    virtual Result      setConfig(const SpiConfig& config)                  = 0;
    virtual AsyncResult writeRead(const Slice& txData, const Slice& rxData) = 0;
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

    virtual Result setTransitionConfig(I2cMasterTransitionConfig& config) = 0;
};

};  // namespace wibot

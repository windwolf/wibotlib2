#pragma once

#include "buffer.hpp"
#include "peripheral.hpp"
#include "wait-handler.hpp"

namespace wibot {

ADC_PER_DECL

union AdcConfig {
    struct {
        u8 channelCount;
        u8 bitWidth;
    };
    u32 value;
};

class Adc : private PeripheralBase, private Initializable {
   public:
    Adc(ADC_CTOR_ARG);
    ~Adc();

    Result read(Buffer32 buffer);
    Result start(Buffer32 buffer, WaitHandler& waitHandler);
    Result stop();

   public:
    AdcConfig config;

   private:
    void _init() override;

   private:
    ADC_FIELD_DECL
    // union {
    //     struct {
    //         bool isRunning : 1;
    //     };
    //     u32 value;
    // } _status;
    WaitTrigger _waitTrigger;
    Buffer32    _buffer;
    static void _onConversionCplt(ADC_CALLBACK_ARG);
    static void _onError(ADC_CALLBACK_ARG);
};

};  // namespace wibot

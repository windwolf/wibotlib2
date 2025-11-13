#pragma once

//
// Created by zhouj on 2023/10/10.
//

#include "type.hpp"
#include "peripheral.hpp"
#

namespace wibot {

using PulseChannel                    = u8;
constexpr PulseChannel kPulseChannel1 = 0x01;
constexpr PulseChannel kPulseChannel2 = 0x04;
constexpr PulseChannel kPulseChannel3 = 0x10;
constexpr PulseChannel kPulseChannel4 = 0x40;

class Pulse : private PeripheralBase, private Initializable {
   public:
    explicit Pulse(TIM_TypeDef* tim, DMA_TypeDef* dma, u32 channel)
        : _handle(tim), _dma(dma), _channel(channel) {};
    Result setDuty(PwmChannel channel, f32 duty) override;
    Result enableChannel(PwmChannel channels) override;
    Result disableChannel(PwmChannel channels) override;
    Result setConfig(PwmConfig& config) override;

   private:
    void _init() override;

   private:
    TIM_TypeDef* _handle;
};

}  // namespace wibot

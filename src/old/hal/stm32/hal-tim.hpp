#pragma once

//
// Created by zhouj on 2023/10/10.
//

#include "pwm.hpp"
#include "type.hpp"
#include "peripheral.hpp"
#

namespace wibot {

class Timer : public PwmGroup, private PeripheralBase, private Initializable {
   public:
    explicit Timer(TIM_HandleTypeDef* handle);
    Result setDuty(PwmChannel channel, f32 duty) override;
    Result enableChannel(PwmChannel channels) override;
    Result disableChannel(PwmChannel channels) override;
    Result setConfig(PwmConfig& config) override;

   private:
    void _init() override;

   private:
    TIM_HandleTypeDef* _handle;
};

}  // namespace wibot

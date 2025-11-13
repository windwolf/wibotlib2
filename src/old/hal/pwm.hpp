#pragma once

#include "type.hpp"

namespace wibot {
#define PWM_PER_DECL

using PwmChannel = u8;

constexpr PwmChannel kPwmChannel1  = 0x01;
constexpr PwmChannel kPwmChannel1N = 0x02;
constexpr PwmChannel kPwmChannel2  = 0x04;
constexpr PwmChannel kPwmChannel2N = 0x08;
constexpr PwmChannel kPwmChannel3  = 0x10;
constexpr PwmChannel kPwmChannel3N = 0x20;
constexpr PwmChannel kPwmChannel4  = 0x40;

struct PwmConfig {};

class PwmGroup {
   public:
    virtual Result setConfig(PwmConfig& config)          = 0;
    virtual Result setDuty(PwmChannel channel, f32 duty) = 0;
    virtual Result enableChannel(PwmChannel channels)    = 0;
    virtual Result disableChannel(PwmChannel channels)   = 0;
};
}  // namespace wibot

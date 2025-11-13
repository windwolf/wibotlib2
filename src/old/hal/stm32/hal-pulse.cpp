// //
// // Created by zhouj on 2023/10/10.
// //

// #include "hal-tim.hpp"

// namespace wibot {

// static u32 convertChannel(PwmChannel channel) {
//     u32 result = 0;
//     if ((channel & kPwmChannel1) == kPwmChannel1) {
//         result |= LL_TIM_CHANNEL_CH1;
//     } else if ((channel & kPwmChannel1N) == kPwmChannel1N) {
//         result |= LL_TIM_CHANNEL_CH1N;
//     } else if ((channel & kPwmChannel2) == kPwmChannel2) {
//         result |= LL_TIM_CHANNEL_CH2;
//     } else if ((channel & kPwmChannel2N) == kPwmChannel2N) {
//         result |= LL_TIM_CHANNEL_CH2N;
//     } else if ((channel & kPwmChannel3) == kPwmChannel3) {
//         result |= LL_TIM_CHANNEL_CH3;
//     } else if ((channel & kPwmChannel3N) == kPwmChannel3N) {
//         result |= LL_TIM_CHANNEL_CH3N;
//     } else if ((channel & kPwmChannel4) == kPwmChannel4) {
//         result |= LL_TIM_CHANNEL_CH4;
//     }
//     return result;
// }

// Pulse::Pulse(TIM_HandleTypeDef* handle) : _handle(handle){};

// void Pulse::_init(){};

// Result Pulse::setDuty(PwmChannel channel, f32 duty) {
//     // NOT add 1 to period, because if CCR value greater than period, the effect just like CCR = 0.
//     // See RM0440: 20.3.8, Figure 302.
//     u32 period = LL_TIM_GetAutoReload(_handle->Instance);

//     if ((channel & (kPwmChannel1 | kPwmChannel1N)) != 0) {
//         LL_TIM_OC_SetCompareCH1(_handle->Instance, static_cast<u32>(period * duty));
//     }
//     if ((channel & (kPwmChannel2 | kPwmChannel2N)) != 0) {
//         LL_TIM_OC_SetCompareCH2(_handle->Instance, static_cast<u32>(period * duty));
//     }
//     if ((channel & (kPwmChannel3 | kPwmChannel3N)) != 0) {
//         LL_TIM_OC_SetCompareCH3(_handle->Instance, static_cast<u32>(period * duty));
//     }
//     if ((channel & kPwmChannel4)) {
//         LL_TIM_OC_SetCompareCH4(_handle->Instance, static_cast<u32>(period * duty));
//     }

//     return Result::kOk;
// };

// Result Timer::enableChannel(PwmChannel channels) {
//     LL_TIM_CC_EnableChannel(_handle->Instance, convertChannel(channels));
//     return Result::kOk;
// };

// Result Timer::disableChannel(PwmChannel channels) {
//     LL_TIM_CC_DisableChannel(_handle->Instance, convertChannel(channels));
//     return Result::kOk;
// }
// Result Timer::setConfig(PwmConfig& config) {
//     return Result::kOk;
// }

// }  // namespace wibot

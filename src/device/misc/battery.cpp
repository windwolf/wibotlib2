//
// Created by zhouj on 2024/1/30.
//

#include "battery.hpp"

namespace wibot {

Battery::Battery(const Battery::Config& config) : config_(config) {
    _100OverSerials = 100.0f / config.serials;
}

u8 Battery::getCapPercent(f32 voltage) {
    switch (config_.type) {
        case Type::kLiPo:
            return getCapPercentLiPo(voltage);
            break;
        case Type::KAA:
            return getCapPercentAA(voltage);
            break;
    }
    return 0;
}
u8 Battery::getCapPercentLiPo(f32 voltage) {
    auto volt100 = (u16)(voltage * _100OverSerials);
    int  thd     = 0;
    for (; thd < 11; ++thd) {
        if (volt100 < lipoThreshold[thd]) {
            break;
        }
    }
    if (thd >= 11) {
        return 100;
    }
    if (thd == 0) {
        return 0;
    }

    return thd * 10 -
           (lipoThreshold[thd] - volt100) * 10 / (lipoThreshold[thd] - lipoThreshold[thd - 1]);
}
u8 Battery::getCapPercentAA(f32 voltage) {
    auto volt100 = (u16)(voltage * _100OverSerials);
    int  thd     = 0;
    for (; thd < 11; ++thd) {
        if (volt100 < aaThreshold[thd]) {
            break;
        }
    }
    if (thd >= 11) {
        return 100;
    }
    if (thd == 0) {
        return 0;
    }

    return thd * 10 - (aaThreshold[thd] - volt100) * 10 / (aaThreshold[thd] - aaThreshold[thd - 1]);
}
}  // namespace wibot

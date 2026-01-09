#pragma once

//
// Created by zhouj on 2024/1/30.
//

#include "type.hpp"

namespace wibot::device {
class Battery {
   public:
    enum class Type {
        kLiPo,
        KAA,
    };

    struct Config {
        Type type;
        u8   serials;
    };

   public:
    Battery(const Config& config);

    /**
     *
     * @param voltage
     * @return 0-10;
     */
    u8 getCapPercent(f32 voltage);

   private:
    Config config_;
    f32    _100OverSerials;

    /**
     * 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100
     */
    constexpr static u16 lipoThreshold[11] = {300, 364, 374, 377, 379, 382,
                                              387, 392, 398, 406, 420};
    constexpr static u16 aaThreshold[11]   = {70, 87, 101, 108, 112, 115, 118, 121, 125, 132, 150};
    u8                   getCapPercentLiPo(f32 voltage);
    u8                   getCapPercentAA(f32 voltage);
};

// MappingFunction<f32, u8> createBatteryCapacityMapper(const Battery::Config& batteryConfig) {
//     Battery battery(batteryConfig);
//     return [battery](f32 voltage, u8 channel) mutable -> u8 {
//         return battery.getCapPercent(voltage);
//     };
// };
}  // namespace wibot

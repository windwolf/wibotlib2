#pragma once

//
// Created by zhouj on 2023/9/27.
//

#include "bus.hpp"

namespace wibot {

using Fxl6408Pins             = u8;
constexpr Fxl6408Pins Pin0    = 0x01;
constexpr Fxl6408Pins Pin1    = 0x02;
constexpr Fxl6408Pins Pin2    = 0x04;
constexpr Fxl6408Pins Pin3    = 0x08;
constexpr Fxl6408Pins Pin4    = 0x10;
constexpr Fxl6408Pins Pin5    = 0x20;
constexpr Fxl6408Pins Pin6    = 0x40;
constexpr Fxl6408Pins Pin7    = 0x80;
constexpr Fxl6408Pins PinAll  = 0xFF;
constexpr Fxl6408Pins PinNone = 0x00;
class Fxl6408I2c {
   public:
    explicit Fxl6408I2c(I2cMaster& i2c, bool isAddr0 = true);

    void reset();

    u8 getId();

    void setDirection(Fxl6408Pins pins, bool isOutput = true);

    void setOutputs(Fxl6408Pins pins, Fxl6408Pins values, Fxl6408Pins highZ);

    Fxl6408Pins getInputs(Fxl6408Pins pins);

    Fxl6408Pins getInterrupts(Fxl6408Pins pins);

    /**
     *
     * @param enables
     * @param pullDownOrUp 0=down, 1=up
     */
    void enablePull(Fxl6408Pins pins, bool isUp = true);
    void disablePull(Fxl6408Pins pins);
    void configInterruptMask(Fxl6408Pins masks);

    void configInputDefault(Fxl6408Pins values);

   private:
    I2cMaster& _i2c;
    bool       _isAddr0;
};

} // namespace wibot


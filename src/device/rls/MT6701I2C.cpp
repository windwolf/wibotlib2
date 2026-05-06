//
// Created by zhouj on 2022/12/2.
//

#include "MT6701I2C.hpp"
#

#define MT6701_I2C_ADDRESS 0x06

#define MT6701_I2C_ANGLE_H8 0x03
#define MT6701_I2C_ANGLE_L6 0x04

namespace wibot {
Mt6701I2c::Mt6701I2c(I2cMaster& i2c) : _i2c(i2c), _value(0) {
    _i2c.setTransitionConfig(MT6701_I2C_ADDRESS);
};

f32 Mt6701I2c::GetAngle() {
    return GetData() / 16384.0f * 360.0f;
}

u32 Mt6701I2c::GetData() {
    u8 h8;
    u8 l6;

    auto   ar    = _i2c.readReg(MT6701_I2C_ANGLE_H8, h8);
    Result rsth8 = ar.wait(TIMEOUT_FOREVER);
    if (rsth8 != Result::kOk) {
        return this->_value;
    }
    ar           = _i2c.readReg(MT6701_I2C_ANGLE_L6, l6);
    Result rstl6 = ar.wait(TIMEOUT_FOREVER);

    if (rstl6 == Result::kOk) {
        this->_value = (h8 << 6) + (l6 >> 2);
    }
    return this->_value;
}

}  // namespace wibot

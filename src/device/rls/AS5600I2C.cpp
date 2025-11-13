//
// Created by zhouj on 2022/12/2.
//

#include "AS5600I2C.hpp"
#

namespace wibot {
As5600I2c::As5600I2c(I2cMaster* i2c) : _i2c(i2c) {
    _i2c->setTransitionConfig(AS5600_I2C_ADDRESS);
};

void As5600I2c::SetZero() {
    const u16 calibrationRound = 500;
    BUFFER(data, 2);

    u32 pos_sum = 0;
    for (int i = 0; i < calibrationRound; i++) {
        auto ar = _i2c->readReg(AS5600_I2C_RAWANGLE, data);
        ar.wait(TIMEOUT_FOREVER);
        u16 pos = (data.data[0] << 8) | data.data[1];
        pos_sum += pos;
        os::sleep(1);
    }
    pos_sum /= calibrationRound;
    data.data[0] = (pos_sum >> 8) & 0xFF;
    data.data[1] = pos_sum & 0xFF;
    auto ar      = _i2c->writeReg(AS5600_I2C_ZPOS, data);
    ar.wait(TIMEOUT_FOREVER);
    ar = _i2c->writeReg(AS5600_I2C_MPOS, data);
    ar.wait(TIMEOUT_FOREVER);
    ar = _i2c->readReg(AS5600_I2C_ANGLE, data);
    ar.wait(TIMEOUT_FOREVER);
}

u32 As5600I2c::GetAngle() {
    BUFFER(data, 2);
    auto ar = _i2c->readReg(AS5600_I2C_ANGLE, data);
    ar.wait(TIMEOUT_FOREVER);
    return (data.data[0] << 8) | data.data[1];
}

u32 As5600I2c::GetData() {
    return GetAngle();
}
u16 As5600I2c::getConfig() {
    BUFFER(data, 2);
    auto ar = _i2c->readReg(AS5600_I2C_CONF, data);
    ar.wait(TIMEOUT_FOREVER);
    return (data.data[0] << 8) | data.data[1];
}
u8 As5600I2c::getStatus() {
    BUFFER(data, 1);
    auto ar = _i2c->readReg(AS5600_I2C_STATUS, data);
    ar.wait(TIMEOUT_FOREVER);
    return data.data[0];
}
u16 As5600I2c::getZpos() {
    BUFFER(data, 2);
    auto ar = _i2c->readReg(AS5600_I2C_ZPOS, data);
    ar.wait(TIMEOUT_FOREVER);
    return (data.data[0] << 8) | data.data[1];
}
u16 As5600I2c::getMpos() {
    BUFFER(data, 2);
    auto ar = _i2c->readReg(AS5600_I2C_MPOS, data);
    ar.wait(TIMEOUT_FOREVER);
    return (data.data[0] << 8) | data.data[1];
}

}  // namespace wibot

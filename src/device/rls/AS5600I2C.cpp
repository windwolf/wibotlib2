//
// Created by zhouj on 2022/12/2.
//

#include "AS5600I2C.hpp"
#

namespace wibot {
As5600I2c::As5600I2c(I2cMaster& i2c) : _i2c(i2c), _angle(0), _config(0), _status(0), _zpos(0), _mpos(0) {
    _i2c.setTransitionConfig(AS5600_I2C_ADDRESS);
};

void As5600I2c::SetZero() {
    const u16 calibrationRound = 500;
    Buffer<2> data{};

    u32 pos_sum     = 0;
    u16 validSample = 0;
    for (int i = 0; i < calibrationRound; i++) {
        auto ar = _i2c.readReg(AS5600_I2C_RAWANGLE, data);
        if (ar.wait(TIMEOUT_FOREVER) == Result::kOk) {
            u16 pos = (data.data[0] << 8) | data.data[1];
            pos_sum += pos;
            validSample++;
        }
        sleep(1);
    }
    if (validSample == 0) {
        return;
    }
    pos_sum /= validSample;
    data.data[0] = (pos_sum >> 8) & 0xFF;
    data.data[1] = pos_sum & 0xFF;
    auto ar      = _i2c.writeReg(AS5600_I2C_ZPOS, data);
    ar.wait(TIMEOUT_FOREVER);
    ar = _i2c.writeReg(AS5600_I2C_MPOS, data);
    ar.wait(TIMEOUT_FOREVER);
    ar = _i2c.readReg(AS5600_I2C_ANGLE, data);
    if (ar.wait(TIMEOUT_FOREVER) == Result::kOk) {
        _angle = (data.data[0] << 8) | data.data[1];
    }
}

u32 As5600I2c::GetAngle() {
    Buffer<2> data{};
    auto      ar = _i2c.readReg(AS5600_I2C_ANGLE, data);
    if (ar.wait(TIMEOUT_FOREVER) == Result::kOk) {
        _angle = (data.data[0] << 8) | data.data[1];
    }
    return _angle;
}

u32 As5600I2c::GetData() {
    return GetAngle();
}
u16 As5600I2c::getConfig() {
    Buffer<2> data{};
    auto      ar = _i2c.readReg(AS5600_I2C_CONF, data);
    if (ar.wait(TIMEOUT_FOREVER) == Result::kOk) {
        _config = (data.data[0] << 8) | data.data[1];
    }
    return _config;
}
u8 As5600I2c::getStatus() {
    u8   data = 0;
    auto ar = _i2c.readReg(AS5600_I2C_STATUS, data);
    if (ar.wait(TIMEOUT_FOREVER) == Result::kOk) {
        _status = data;
    }
    return _status;
}
u16 As5600I2c::getZpos() {
    Buffer<2> data{};
    auto      ar = _i2c.readReg(AS5600_I2C_ZPOS, data);
    if (ar.wait(TIMEOUT_FOREVER) == Result::kOk) {
        _zpos = (data.data[0] << 8) | data.data[1];
    }
    return _zpos;
}
u16 As5600I2c::getMpos() {
    Buffer<2> data{};
    auto      ar = _i2c.readReg(AS5600_I2C_MPOS, data);
    if (ar.wait(TIMEOUT_FOREVER) == Result::kOk) {
        _mpos = (data.data[0] << 8) | data.data[1];
    }
    return _mpos;
}

}  // namespace wibot

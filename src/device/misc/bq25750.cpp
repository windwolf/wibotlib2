#include "bq25750.hpp"
namespace wibot {
Bq25750::Bq25750(I2cMaster* i2c) : _i2c(i2c) {
    _i2c->setTransitionConfig(Bq25750::I2C_ADDRESS);
}

Bq25750::~Bq25750() {
}

Bq25750::State Bq25750::getState() {
    Buffer<4> data;
    auto      ar = _i2c->readReg(0x21, data);
    ar.wait(TIMEOUT_NOWAIT);

    Bq25750::State sta;
    sta.chargerStatus1.raw = data[0];
    sta.chargerStatus2.raw = data[1];
    sta.chargerStatus3.raw = data[2];
    sta.faultStatus.raw    = data[3];

    return sta;
};

Bq25750::IntFlag Bq25750::getFlag() {
    Buffer<3> data;
    auto      ar = _i2c->readReg(0x25, data);
    ar.wait(TIMEOUT_NOWAIT);

    Bq25750::IntFlag flag;
    flag.intFlag1.raw  = data[0];
    flag.intFlag2.raw  = data[1];
    flag.faultFlag.raw = data[2];

    return flag;
};

void Bq25750::setMask(Bq25750::IntMask mask) {
    Buffer<3> data;
    data[0] = mask.intMask1.raw;
    data[1] = mask.intMask2.raw;
    data[2] = mask.faultMask.raw;

    auto ar = _i2c->writeReg(0x28, data);
    ar.wait(TIMEOUT_NOWAIT);
};

Result Bq25750::enableCharging() {
    Buffer<1> data;

    auto rst = _i2c->readReg(0x17, data).wait(TIMEOUT_NOWAIT);
    if (!rst.isOk()) {
        return rst;
    }
    data[0] |= 0x01;
    return _i2c->writeReg(0x17, data).wait(TIMEOUT_NOWAIT);
};

Result Bq25750::disableCharging() {
    Buffer<1> data;

    auto rst = _i2c->readReg(0x17, data).wait(TIMEOUT_NOWAIT);
    if (!rst.isOk()) {
        return rst;
    }
    data[0] &= 0xFE;
    return _i2c->writeReg(0x17, data).wait(TIMEOUT_NOWAIT);
};

Result Bq25750::feedDog() {
    Buffer<1> data;

    auto rst = _i2c->readReg(0x17, data).wait(TIMEOUT_NOWAIT);
    if (!rst.isOk()) {
        return rst;
    }

    data[0] |= 0x20;
    return _i2c->writeReg(0x17, data).wait(TIMEOUT_NOWAIT);
};

}  // namespace wibot
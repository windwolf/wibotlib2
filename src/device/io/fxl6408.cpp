//
// Created by zhouj on 2023/9/27.
//

#include "fxl6408.hpp"
#include "buffer.hpp"

namespace wibot {
Fxl6408I2c::Fxl6408I2c(I2cMaster* i2c, bool isAddr0) : _i2c(i2c), _isAddr0(isAddr0) {
    _i2c->setTransitionConfig(_isAddr0 ? 0x43 : 0x44);
}
void Fxl6408I2c::reset() {
    BUFFER(ids, 1);
    auto ar = _i2c->readReg(0x01, ids);
    ar.wait(TIMEOUT_FOREVER);

    ids.data[0] |= 0x01;
    ar = _i2c->writeReg(0x01, ids);
    ar.wait(TIMEOUT_FOREVER);
};
u8 Fxl6408I2c::getId() {
    BUFFER(ids, 1);
    auto ar = _i2c->readReg(0x01, ids);
    ar.wait(TIMEOUT_FOREVER);
    return ids.data[0];
}
void Fxl6408I2c::setDirection(Fxl6408Pins pins, bool isOutput) {
    BUFFER(v, 1);
    auto ar = _i2c->readReg(0x03, v);
    ar.wait(TIMEOUT_FOREVER);
    if (isOutput) {
        v.data[0] |= pins;
    } else {
        v.data[0] &= ~pins;
    }
    ar = _i2c->writeReg(0x03, v);
    ar.wait(TIMEOUT_FOREVER);
}
void Fxl6408I2c::setOutputs(Fxl6408Pins pins, Fxl6408Pins values, Fxl6408Pins highZ) {
    BUFFER(o, 1);
    BUFFER(h, 1);

    auto ar = _i2c->readReg(0x05, o);
    ar.wait(TIMEOUT_FOREVER);
    ar = _i2c->readReg(0x07, h);
    ar.wait(TIMEOUT_FOREVER);

    h.data[0] &= ~pins;
    h.data[0] |= pins & highZ;
    ar = _i2c->writeReg(0x07, h);
    ar.wait(TIMEOUT_FOREVER);
    o.data[0] &= ~pins;
    o.data[0] |= pins & values;
    ar = _i2c->writeReg(0x05, o);
    ar.wait(TIMEOUT_FOREVER);
}
Fxl6408Pins Fxl6408I2c::getInputs(Fxl6408Pins pins) {
    BUFFER(i, 1);
    auto ar = _i2c->readReg(0x0F, i);
    ar.wait(TIMEOUT_FOREVER);

    return i.data[0] & pins;
}
Fxl6408Pins Fxl6408I2c::getInterrupts(Fxl6408Pins pins) {
    BUFFER(i, 1);
    auto ar = _i2c->readReg(0x13, i);
    ar.wait(TIMEOUT_FOREVER);
    return i.data[0] & pins;
}
void Fxl6408I2c::enablePull(Fxl6408Pins pins, bool isUp) {
    BUFFER(e, 1);
    BUFFER(u, 1);
    auto ar = _i2c->readReg(0x0B, e);
    ar.wait(TIMEOUT_FOREVER);
    e.data[0] |= pins;
    ar = _i2c->writeReg(0x0B, e);
    ar.wait(TIMEOUT_FOREVER);

    ar = _i2c->readReg(0x0D, u);
    ar.wait(TIMEOUT_FOREVER);
    if (isUp) {
        u.data[0] |= pins;
    } else {
        u.data[0] &= ~pins;
    }
    ar = _i2c->writeReg(0x0D, u);
    ar.wait(TIMEOUT_FOREVER);
}
void Fxl6408I2c::disablePull(Fxl6408Pins pins) {
    BUFFER(e, 1);
    auto ar = _i2c->readReg(0x0B, e);
    ar.wait(TIMEOUT_FOREVER);
    e.data[0] &= ~pins;
    _i2c->writeReg(0x0B, e);
    ar.wait(TIMEOUT_FOREVER);
}
void Fxl6408I2c::configInterruptMask(Fxl6408Pins masks) {
    auto ar = _i2c->writeReg(0x11, Slice(&masks, 1));
    ar.wait(TIMEOUT_FOREVER);
}
void Fxl6408I2c::configInputDefault(Fxl6408Pins values) {
    auto ar = _i2c->writeReg(0x09, Slice(&values, 1));
    ar.wait(TIMEOUT_FOREVER);
}
}  // namespace wibot

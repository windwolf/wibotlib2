//
// Created by zhouj on 2023/9/27.
//

#include "fxl6408.hpp"
#include "buffer.hpp"

namespace wibot {
Fxl6408I2c::Fxl6408I2c(I2cMaster& i2c, bool isAddr0) : _i2c(i2c), _isAddr0(isAddr0) {
    _i2c.setTransitionConfig(_isAddr0 ? 0x43 : 0x44);
}
void Fxl6408I2c::reset() {
    u8   ids;
    auto ar = _i2c.readReg(0x01, Slice(&ids, 1));
    ar.wait(TIMEOUT_FOREVER);

    ids |= 0x01;
    ar = _i2c.writeReg(0x01, Slice(&ids, 1));
    ar.wait(TIMEOUT_FOREVER);
};
u8 Fxl6408I2c::getId() {
    u8   ids;
    auto ar = _i2c.readReg(0x01, ids);
    ar.wait(TIMEOUT_FOREVER);
    return ids;
}
void Fxl6408I2c::setDirection(Fxl6408Pins pins, bool isOutput) {
    u8   v;
    auto ar = _i2c.readReg(0x03, v);
    ar.wait(TIMEOUT_FOREVER);
    if (isOutput) {
        v |= pins;
    } else {
        v &= ~pins;
    }
    ar = _i2c.writeReg(0x03, v);
    ar.wait(TIMEOUT_FOREVER);
}
void Fxl6408I2c::setOutputs(Fxl6408Pins pins, Fxl6408Pins values, Fxl6408Pins highZ) {
    u8 o;
    u8 h;

    auto ar = _i2c.readReg(0x05, o);
    ar.wait(TIMEOUT_FOREVER);
    ar = _i2c.readReg(0x07, h);
    ar.wait(TIMEOUT_FOREVER);

    h &= ~pins;
    h |= pins & highZ;
    ar = _i2c.writeReg(0x07, h);
    ar.wait(TIMEOUT_FOREVER);
    o &= ~pins;
    o |= pins & values;
    ar = _i2c.writeReg(0x05, o);
    ar.wait(TIMEOUT_FOREVER);
}
Fxl6408Pins Fxl6408I2c::getInputs(Fxl6408Pins pins) {
    u8   i;
    auto ar = _i2c.readReg(0x0F, i);
    ar.wait(TIMEOUT_FOREVER);

    return i & pins;
}
Fxl6408Pins Fxl6408I2c::getInterrupts(Fxl6408Pins pins) {
    u8   i;
    auto ar = _i2c.readReg(0x13, i);
    ar.wait(TIMEOUT_FOREVER);
    return i & pins;
}
void Fxl6408I2c::enablePull(Fxl6408Pins pins, bool isUp) {
    u8   e;
    u8   u;
    auto ar = _i2c.readReg(0x0B, e);
    ar.wait(TIMEOUT_FOREVER);
    e |= pins;
    ar = _i2c.writeReg(0x0B, e);
    ar.wait(TIMEOUT_FOREVER);

    ar = _i2c.readReg(0x0D, u);
    ar.wait(TIMEOUT_FOREVER);
    if (isUp) {
        u |= pins;
    } else {
        u &= ~pins;
    }
    ar = _i2c.writeReg(0x0D, u);
    ar.wait(TIMEOUT_FOREVER);
}
void Fxl6408I2c::disablePull(Fxl6408Pins pins) {
    u8   e;
    auto ar = _i2c.readReg(0x0B, e);
    ar.wait(TIMEOUT_FOREVER);
    e &= ~pins;
    ar = _i2c.writeReg(0x0B, e);
    ar.wait(TIMEOUT_FOREVER);
}
void Fxl6408I2c::configInterruptMask(Fxl6408Pins masks) {
    auto ar = _i2c.writeReg(0x11, Slice(&masks, 1));
    ar.wait(TIMEOUT_FOREVER);
}
void Fxl6408I2c::configInputDefault(Fxl6408Pins values) {
    auto ar = _i2c.writeReg(0x09, Slice(&values, 1));
    ar.wait(TIMEOUT_FOREVER);
}
}  // namespace wibot

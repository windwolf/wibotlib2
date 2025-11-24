//
// Created by zhouj on 2024/11/18.
//

#include "xw09a.hpp"
#include "buffer.hpp"

namespace wibot {

XW09A::XW09A(I2cMaster* i2c) : _i2c(i2c) {
}

u16 XW09A::refreshState() {
    Buffer<2> data;
    auto ar = _i2c->readReg(0x00, data);
    ar.wait(TIMEOUT_FOREVER);
    _state = data.data[0] << 8 | data.data[1];
    return _state;
}

bool XW09A::isPadPressed(u8 padNum) {
    u16 state = _state;
    return !(state & (1 << (padNum + 12)));
}

}  // namespace wibot

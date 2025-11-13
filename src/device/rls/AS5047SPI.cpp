//
// Created by zhouj on 2023/2/28.
//

#include "AS5047SPI.hpp"

namespace wibot {

u32 As5047Spi::getAngle() {
    _cmd[0] = AS5047_CMD_READ_ANGLECOM;
    _cmd[1] = AS5047_CMD_READ_ANGLECOM;
    _spi->begin();
    auto ar = _spi->writeRead(Slice((u8*)_cmd, 4), Slice((u8*)_cmd, 4));
    _spi->end();
    _parity.reset();
    ar.wait(TIMEOUT_FOREVER);
    _parity.calculate(static_cast<u8*>(static_cast<void*>(&_cmd[1])), 2);
    if (_parity.validate(nullptr)) {
        _angle = _cmd[1] & 0x3FFF;
        if (!(_cmd[1] & AS5047_EF_BIT)) {
            _state.invalidData = true;
        }
        _state.parityError = false;
    } else {
        _state.parityError = true;
    }
    return _angle;
};
u32 As5047Spi::getData() {
    return getAngle();
};
}  // namespace wibot

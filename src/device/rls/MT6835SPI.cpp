//
// Created by zhouj on 2023/2/12.
//

#include "MT6835SPI.hpp"

namespace wibot {

#ifdef HAL_SPI_MODULE_ENABLED

#define MT6835SPI_READ_CMD  0xA0
#define MT6835SPI_ANGLE_REG 0x03

u32 Mt6835Spi::getAngle() {
    _buf[0] = MT6835SPI_READ_CMD;
    _buf[1] = MT6835SPI_ANGLE_REG;
    _spi.begin();
    auto ar = _spi.writeRead(Slice(_buf, 6), Slice(_buf, 6));
    _spi.end();
    _crc.reset();
    ar.wait(TIMEOUT_FOREVER);
    _crc.calculate(_buf + 2, 3);
    if (_crc.validate(_buf + 5)) {
        _angle             = (_buf[2] << 13) | (_buf[3] << 5) | (_buf[4] >> 3);
        _state.overSpeed   = _buf[4] & 0x01;
        _state.weakMagnet  = _buf[4] & 0x02;
        _state.overVoltage = _buf[4] & 0x04;
        _state.crcError    = false;
    } else {
        _state.crcError = true;
    }
    return _angle;
}
u32 Mt6835Spi::GetData() {
    return getAngle();
}

#endif  // HAL_SPI_MODULE_ENABLED

}  // namespace wibot

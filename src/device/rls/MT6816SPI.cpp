//
// Created by zhoujian on 2022/12/12.
//

#include "MT6816SPI.hpp"
#ifdef HAL_SPI_MODULE_ENABLED

#define MT6816_SPI_READ_ANGLE1_REG 0x83
#define MT6816_SPI_READ_ANGLE2_REG 0x84
#define MT6816_SPI_READ_ANGLE3_REG 0x85

#define MT6816_SPI_ANGLE_BIT_MASK      0xF8
#define MT6816_SPI_NO_MAG_BIT_MASK     0x02
#define MT6816_SPI_PC_BIT_MASK         0x01
#define MT6816_SPI_OVER_SPEED_BIT_MASK 0x18

namespace wibot {

u32 Mt6816Spi::getAngle() {
    u16 angle = 0;
    _cmd[0]   = MT6816_SPI_READ_ANGLE1_REG;
    _cmd[1]   = 0;
    _cmd[2]   = 0;
    Result rst = _spi.begin();
    if (rst != Result::kOk) {
        return _angle;
    }
    auto ar = _spi.writeRead(Slice(_cmd, 3), Slice(_cmd, 3));
    _parity.reset();
    rst = ar.wait(TIMEOUT_FOREVER);
    _spi.end();
    if (rst != Result::kOk) {
        return _angle;
    }
    _parity.calculate(&_cmd[1], 2);
    if (_parity.validate(nullptr)) {
        angle = _cmd[1] << 6;
        angle |= _cmd[2] >> 2;
        _angle = angle;
    }
    return _angle;
}

u32 Mt6816Spi::getData() {
    return getAngle();
}
}  // namespace wibot

#endif

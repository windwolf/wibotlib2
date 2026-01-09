//
// Created by zhouj on 2023/1/3.
//

#include "MT6825SPI.hpp"
#ifdef HAL_SPI_MODULE_ENABLED

#define MT6825_SPI_READ_CMD   0x80
#define MT6825_SPI_WRITE_CMD  0x00
#define MT6825_SPI_ANGLE1_REG 0x03
#define MT6825_SPI_ANGLE2_REG 0x04
#define MT6825_SPI_ANGLE3_REG 0x05

#define MT6825_SPI_ANGLE_BIT_MASK      0xF8
#define MT6825_SPI_NO_MAG_BIT_MASK     0x02
#define MT6825_SPI_PC_BIT_MASK         0x01
#define MT6825_SPI_OVER_SPEED_BIT_MASK 0x18

namespace wibot::device {

u32 Mt6825Spi::GetAngle() {
    _cmd[0] = MT6825_SPI_ANGLE1_REG | MT6825_SPI_READ_CMD;
    _spi.begin();
    auto ar = _spi.writeRead(Slice(_cmd, 4), Slice(_cmd, 4));
    _spi.end();
    ar.wait(TIMEOUT_FOREVER);
    return (_cmd[1] << 10) | ((_cmd[2] & 0xfc) << 2) | (_cmd[3] >> 4);
}
u32 Mt6825Spi::GetData() {
    return GetAngle();
}
}  // namespace wibot

#endif

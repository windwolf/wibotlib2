#pragma once

//
// Created by zhouj on 2023/2/12.
//

#include "rls.hpp"
#include "chip.hpp"
#include "bus.hpp"
#include "comm/crc/crc8.hpp"

namespace wibot {

#ifdef HAL_SPI_MODULE_ENABLED

struct MT6835State {
    bool overSpeed   : 1;
    bool weakMagnet  : 1;
    bool overVoltage : 1;
    bool crcError    : 1;
};

struct MT6835SpiConfig {};

class Mt6835Spi {
   public:
    Mt6835Spi(SpiMaster& spi) : _spi(spi), _angle(0), _state{}, _crc(0x07) {};

    u32 getAngle();

    u32 GetData();

    MT6835State GetState() const {
        return _state;
    }

    u32 getResolution() const {
        return 1 << 21;
    }

   private:
    SpiMaster&    _spi;
    u8            _buf[6];
    u32           _angle;
    MT6835State   _state;
    Crc8Validator _crc;
};

#endif

}  // namespace wibot

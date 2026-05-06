#pragma once

//
// Created by zhouj on 2023/1/3.
//

#include "rls.hpp"
#include "chip.hpp"
#include "bus.hpp"

#ifdef HAL_SPI_MODULE_ENABLED
namespace wibot {

struct MT6825SpiConfig {};

class Mt6825Spi {
   public:
    Mt6825Spi(SpiMaster& spi) : _spi(spi), _angle(0) {};

    u32 GetAngle();

    u32 GetData();

   private:
    SpiMaster& _spi;
    u8         _cmd[4];
    u32        _angle;
};

} // namespace wibot

#endif


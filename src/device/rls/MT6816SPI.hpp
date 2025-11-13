#pragma once

#include "rls.hpp"
#include "chip.hpp"
#include "bus.hpp"
#include "parity.hpp"

#ifdef HAL_SPI_MODULE_ENABLED

namespace wibot {

struct Mt6816SpiConfig {};

/**
 * @note 使用前，确保时钟极性为极性为1，上升沿采样，8位
 */
class Mt6816Spi : public Rls {
   public:
    Mt6816Spi(SpiMaster* spi) : _spi(spi), _parity(true) {};

    u32 getAngle() override;

    u32 getData() override;

   private:
    SpiMaster*      _spi;
    u8              _cmd[4];
    u16             _angle;
    ParityValidator _parity;
};

}  // namespace wibot

#endif  // HAL_SPI_MODULE_ENABLED

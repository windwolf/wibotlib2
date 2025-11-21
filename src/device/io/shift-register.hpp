#pragma once

#include "bus.hpp"
#include "gpio.hpp"

namespace wibot {
class SinPoutShiftRegister {
   public:
    SinPoutShiftRegister(SpiMaster* spi, Pin& stcpPin);

    Result write(Slice data);

   private:
    SpiMaster* _spi;
    Pin*       _stcpPin;
};

class PinSoutShiftRegister {
   public:
    PinSoutShiftRegister(SpiMaster* spi, Pin& plPin);

    Result read(const Slice& data);

   private:
    SpiMaster* _spi;
    Pin*       _plPin;
};

}  // namespace wibot

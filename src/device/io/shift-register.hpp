#pragma once

#include "bus.hpp"
#include "hal/stm32/gpio.hpp"

namespace wibot::device {
class SinPoutShiftRegister {
   public:
    SinPoutShiftRegister(SpiMaster& spi, hal::Pin& stcpPin);

    Result write(Slice data);

   private:
    SpiMaster& _spi;
    hal::Pin*  _stcpPin;
};

class PinSoutShiftRegister {
   public:
    PinSoutShiftRegister(SpiMaster& spi, hal::Pin& plPin);

    Result read(const Slice& data);

   private:
    SpiMaster& _spi;
    hal::Pin*  _plPin;
};

}  // namespace wibot::device

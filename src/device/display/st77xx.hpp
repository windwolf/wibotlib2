#pragma once

#include "type.hpp"
#include "bus.hpp"
#include "hal/stm32/gpio.hpp"

namespace wibot {

struct St77xxConfig {
    u16 width;
    u16 height;
    u8  colorMode;
    u8  orientation;
    u16 xOffset;
    u16 yOffset;
};

class St77xx {
   public:
    St77xx(SpiMaster &spi, Pin &dcPin);

   protected:
    u8 _cmdData[16];
    u8 _pvGamma[16];
    u8 _nvGamma[16];

    Result sendCommand(u8 cmdId);
    Result sendCommandData(u8 cmdId, Slice &data, bool isWrite);

    Result sendReadCommand(u8 cmdId, Slice &data);
    Result sendWriteCommand(u8 cmdId, const Slice &data);

   public:
    St77xxConfig config;

   private:
    SpiMaster &_spi;
    Pin       &_dcPin;
};

}  // namespace wibot

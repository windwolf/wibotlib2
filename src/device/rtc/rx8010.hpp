#pragma once

#include "chrono.hpp"
#include "bus.hpp"

namespace wibot {

class Rx8010 {
   public:
    Rx8010(I2cMaster *i2c);

    Result porInit();
    Result getDateTime(DateTime &datetime);
    Result setDateTime(const DateTime &datetime);



   private:
    I2cMaster *_i2c;
    Result        _readI2c(u16 address, void *data, u32 dataSize);
    Result        _writeI2c(u16 address, void *data, u32 dataSize);
};

}  // namespace wibot

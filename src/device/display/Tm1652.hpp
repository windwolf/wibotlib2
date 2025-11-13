#pragma once

#include "bus.hpp"

using namespace wibot;

class Tm1652 {
   public:
    Tm1652(UartStream* uart) : _uart(uart) {
    }
    ~Tm1652();

    /**
     * @brief 
     * 
     * @param brightness grid brightness: 0-15
     * @param current sig driver current: 0-7
     * @param mode  0x00: 8sig/5grid, 0x01: 7sig/6grid
     * @param waitHandler 
     */
    AsyncResult setup(u8 brightness, u8 current, u8 mode);

    AsyncResult updateDisplay(u8* data, u8 length, u8 startGrid);
    AsyncResult updateDisplay(u8* data, u8 length);

   private:
    UartStream* _uart;
    u8          _cmdBuf[7];
};

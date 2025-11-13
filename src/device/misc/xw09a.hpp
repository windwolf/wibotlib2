#pragma once

//
// Created by zhouj on 2024/11/18.
//

#include "bus.hpp"
namespace wibot {

class XW09A {
   public:
    const static constexpr u8 I2C_ADDR = 0x40;

   public:
    XW09A(I2cMaster* i2c);

    /**
     * @return A bitwise combination of the 10 pads' state.
     */
    u16 refreshState();

    /**
     * @param padNum 0-8
     * @return true if the pad is pressed.
     */
    bool isPadPressed(u8 padNum);

   private:
    I2cMaster* _i2c;
    u16           _state;
};

}  // namespace wibot

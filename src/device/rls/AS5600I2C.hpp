#pragma once

//
// Created by zhouj on 2022/12/2.
//

#include "type.hpp"
#include "bus.hpp"

#define AS5600_I2C_ADDRESS   0x36
#define AS5600_I2C_ZPOS      0x01
#define AS5600_I2C_MPOS      0x03
#define AS5600_I2C_MANG      0x05
#define AS5600_I2C_CONF      0x07
#define AS5600_I2C_RAWANGLE  0x0C
#define AS5600_I2C_ANGLE     0x0E
#define AS5600_I2C_STATUS    0x0B
#define AS5600_I2C_AGC       0x1A
#define AS5600_I2C_MAGNITUDE 0x1B
#define AS5600_I2C_BURN      0xFF

namespace wibot {

struct As5600I2cConfig {};
class As5600I2c {
   public:
    As5600I2c(I2cMaster* i2c);

    void SetZero();

    u16 getConfig();
    u8  getStatus();
    u16 getZpos();
    u16 getMpos();

    u32 GetAngle();

    u32 GetData();


   private:
    I2cMaster* _i2c;
};
}  // namespace wibot

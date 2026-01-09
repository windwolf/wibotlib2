#pragma once

//
// Created by zhouj on 2022/12/2.
//

#include "rls.hpp"
#include "chip.hpp"
#include "bus.hpp"

namespace wibot {

struct Mt6701I2cConfig {};
class Mt6701I2c {
   public:
    Mt6701I2c(I2cMaster& i2c);

    u16 getConfig();

    f32 GetAngle();

    u32 GetData();

   private:
    I2cMaster& _i2c;
    u32        _value;
};

} // namespace wibot


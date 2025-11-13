#pragma once

//
// Created by zhouj on 2023/8/30.
//

#include "type.hpp"
#include "chrono.hpp"

namespace wibot {

class Rtc {
   public:
    virtual Result read(DateTime& datetime) = 0;
    virtual Result write(DateTime datetime) = 0;
};

}  // namespace wibot

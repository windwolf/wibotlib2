#pragma once

//
// Created by zhouj on 2022/11/21.
//

#include "type.hpp"

namespace wibot {

class ValueMapper {
   public:
    virtual f32 getValue(u32 inValue) = 0;
};

}  // namespace wibot

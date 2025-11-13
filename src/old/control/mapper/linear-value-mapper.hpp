#pragma once

//
// Created by zhouj on 2022/11/16.
//

#include "value-mapper.hpp"

namespace wibot {

struct LinearValueMapperConfig {
    u32 zeroOffset;
    f32 valuePerUnit;
};

class LinearValueMapper : public ValueMapper {
   public:
    f32 getValue(u32 in_value);

   public:
    LinearValueMapperConfig config;
};
}  // namespace wibot

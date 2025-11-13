//
// Created by zhouj on 2022/11/16.
//

#include "linear-value-mapper.hpp"

namespace wibot {

f32 LinearValueMapper::getValue(u32 raw_value) {
    return (f32)(i32)(raw_value - config.zeroOffset) * config.valuePerUnit;
}

}  // namespace wibot

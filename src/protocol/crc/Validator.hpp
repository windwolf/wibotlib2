#pragma once

//
// Created by zhouj on 2023/4/1.
//

#include "type.hpp"

namespace wibot {

template <typename TE>
class Validator {
   public:
    virtual void reset()                         = 0;
    virtual void calculate(TE* data, u32 length) = 0;
    virtual bool validate(TE* sum)               = 0;
};

}  // namespace wibot

#pragma once

//
// Created by zhouj on 2023/2/20.
//

#include "Validator.hpp"
#include "type.hpp"

namespace wibot::comm {

class CheckSum8Validator : Validator<u8> {
   public:
    void reset() override;
    void calculate(u8* data, u32 length) override;
    bool validate(u8* sum) override;

   private:
    u8 _sum;
};

}  // namespace wibot

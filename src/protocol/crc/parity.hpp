#pragma once

//
// Created by zhouj on 2023/2/21.
//

#include "Validator.hpp"
#include "type.hpp"

namespace wibot {

class ParityValidator : Validator<u8> {
   public:
    /**
     * @brief Construct a new check Parity Validator object
     * @param odd 是否偶校验
     */
    explicit ParityValidator(bool even = false) : _parity(0), _even(even) {};
    void reset() override;
    void calculate(u8* data, u32 length) override;
    /**
     *
     * @param sum Not used.
     * @return
     */
    bool validate(u8* sum) override;

   private:
    u8   _parity;  //
    bool _even;    // 是否偶校验
};

}  // namespace wibot

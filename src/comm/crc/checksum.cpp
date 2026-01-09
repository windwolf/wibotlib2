//
// Created by zhouj on 2023/2/20.
//

#include "checksum.hpp"

namespace wibot {

void CheckSum8Validator::reset() {
    _sum = 0;
}

void CheckSum8Validator::calculate(u8* data, u32 length) {
    for (u32 i = 0; i < length; i++) {
        _sum += data[i];
    }
}

bool CheckSum8Validator::validate(u8* sum) {
    return sum[0] == _sum;
}

}  // namespace wibot

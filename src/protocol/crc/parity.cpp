//
// Created by zhouj on 2023/2/21.
//

#include "parity.hpp"

namespace wibot {
void ParityValidator::reset() {
    _parity = 0;
}
void ParityValidator::calculate(u8* data, u32 length) {
    for (u32 i = 0; i < length; i++) {
        _parity = _parity ^ data[i];
    }
}
bool ParityValidator::validate(u8* sum) {
    bool parity = _even;
    while (_parity) {
        parity = !parity;
        _parity &= _parity - 1;
    }
    return parity;
}
}  // namespace wibot

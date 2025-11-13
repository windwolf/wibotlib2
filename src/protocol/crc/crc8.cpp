//
// Created by zhouj on 2023/2/21.
//

#include "crc8.hpp"

namespace wibot {

void Crc8Validator::reset() {
    _crc = _init;
}
void Crc8Validator::calculate(u8* data, u32 length) {
    for (u32 i = 0; i < length; i++) {
        u8 d = data[i];
        if (_ref_in) {
            d = (d & 0xF0) >> 4 | (d & 0x0F) << 4;
            d = (d & 0xCC) >> 2 | (d & 0x33) << 2;
            d = (d & 0xAA) >> 1 | (d & 0x55) << 1;
        }
        _crc ^= d;
        for (u8 j = 0; j < 8; j++) {
            if (_crc & 0x80) {
                _crc = (_crc << 1) ^ _poly;
            } else {
                _crc <<= 1;
            }
        }
    }
}
bool Crc8Validator::validate(u8* sum) {
    u8 s = *sum;
    if (_ref_out) {
        s = (s & 0xF0) >> 4 | (s & 0x0F) << 4;
        s = (s & 0xCC) >> 2 | (s & 0x33) << 2;
        s = (s & 0xAA) >> 1 | (s & 0x55) << 1;
    }
    return s == (_crc ^ _xorout);
}
u8 Crc8Validator::get() {
    return _crc ^ _xorout;
}
//    void Crc8::init_table()
//    {
//        for (u16 i = 0; i < 256; i++)
//        {
//            u8 crc = i;
//            for (u8 j = 0; j < 8; j++)
//            {
//                if (crc & 0x80)
//                {
//                    crc = (crc << 1) ^ poly_;
//                }
//                else
//                {
//                    crc <<= 1;
//                }
//            }
//            table_[i] = crc;
//        }
//    }
}  // namespace wibot

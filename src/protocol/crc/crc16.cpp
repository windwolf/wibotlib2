//
// Created by AI Assistant on 2025/7/11.
//

#include "crc16.hpp"

namespace wibot {

void Crc16Validator::reset() {
    _crc = _init;
}

void Crc16Validator::calculate(u8* data, u32 length) {
    for (u32 i = 0; i < length; i++) {
        u8 d = data[i];
        if (_ref_in) {
            d = (d & 0xF0) >> 4 | (d & 0x0F) << 4;
            d = (d & 0xCC) >> 2 | (d & 0x33) << 2;
            d = (d & 0xAA) >> 1 | (d & 0x55) << 1;
        }
        _crc ^= (d << 8);
        for (u8 j = 0; j < 8; j++) {
            if (_crc & 0x8000) {
                _crc = (_crc << 1) ^ _poly;
            } else {
                _crc <<= 1;
            }
        }
    }
}

bool Crc16Validator::validate(u8* sum) {
    // Assume sum points to 2 bytes in big-endian format
    u16 s = (sum[0] << 8) | sum[1];
    if (_ref_out) {
        s = (s & 0xFF00) >> 8 | (s & 0x00FF) << 8;
        s = (s & 0xF0F0) >> 4 | (s & 0x0F0F) << 4;
        s = (s & 0xCCCC) >> 2 | (s & 0x3333) << 2;
        s = (s & 0xAAAA) >> 1 | (s & 0x5555) << 1;
    }
    return s == (_crc ^ _xorout);
}

u16 Crc16Validator::get() {
    u16 result = _crc ^ _xorout;
    if (_ref_out) {
        result = (result & 0xFF00) >> 8 | (result & 0x00FF) << 8;
        result = (result & 0xF0F0) >> 4 | (result & 0x0F0F) << 4;
        result = (result & 0xCCCC) >> 2 | (result & 0x3333) << 2;
        result = (result & 0xAAAA) >> 1 | (result & 0x5555) << 1;
    }
    return result;
}

}  // namespace wibot

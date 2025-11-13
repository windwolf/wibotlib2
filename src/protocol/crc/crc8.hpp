#pragma once

//
// Created by zhouj on 2023/2/21.
//

#include "type.hpp"
#include "Validator.hpp"

namespace wibot {

class Crc8Validator : Validator<u8> {
   public:
    Crc8Validator(u8 poly, u8 init = 0x00, u8 xorout = 0x00, bool ref_in = false,
                  bool ref_out = false)
        : _poly(poly), _init(init), _xorout(xorout), _ref_in(ref_in), _ref_out(ref_out) {};
    void reset() override;
    void calculate(u8* data, u32 length) override;
    bool validate(u8* sum) override;
    u8   get();

   private:
    u8   _poly;
    u8   _init;
    u8   _xorout;
    bool _ref_in;
    bool _ref_out;
    u8   _crc;

   public:
    constexpr static u8 CRC8_DVB_S2       = 0xD5;
    constexpr static u8 CRC8_AUTOSAR      = 0x2F;
    constexpr static u8 CRC8_BLUETOOTH    = 0xA7;
    constexpr static u8 CRC8_CCITT        = 0x07;
    constexpr static u8 CRC8_DALLAS_MAXIM = 0x31;
    constexpr static u8 CRC8_DARC         = 0x39;
    constexpr static u8 CRC8_GSM_B        = 0x49;
    constexpr static u8 CRC8_SAE_J1850    = 0x1D;
    constexpr static u8 CRC8_WCDMA        = 0x9B;
    constexpr static u8 CRC8_GSM_A        = 0x37;
};

}  // namespace wibot

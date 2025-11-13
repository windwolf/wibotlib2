#pragma once

//
// Created by AI Assistant on 2025/7/11.
//

#include "type.hpp"
#include "Validator.hpp"

namespace wibot {

class Crc16Validator : Validator<u8> {
   public:
    Crc16Validator(u16 poly, u16 init = 0x0000, u16 xorout = 0x0000, bool ref_in = false,
                   bool ref_out = false)
        : _poly(poly), _init(init), _xorout(xorout), _ref_in(ref_in), _ref_out(ref_out) {};
    void reset() override;
    void calculate(u8* data, u32 length) override;
    bool validate(u8* sum) override;
    u16  get();

   private:
    u16  _poly;
    u16  _init;
    u16  _xorout;
    bool _ref_in;
    bool _ref_out;
    u16  _crc;

   public:
    constexpr static u16 CRC16_CCITT    = 0x1021;
    constexpr static u16 CRC16_ARC      = 0x8005;
    constexpr static u16 CRC16_BUYPASS  = 0x8005;
    constexpr static u16 CRC16_DDS_110  = 0x8005;
    constexpr static u16 CRC16_DECT     = 0x0589;
    constexpr static u16 CRC16_DNP      = 0x3D65;
    constexpr static u16 CRC16_EN_13757 = 0x3D65;
    constexpr static u16 CRC16_GENIBUS  = 0x1021;
    constexpr static u16 CRC16_MAXIM    = 0x8005;
    constexpr static u16 CRC16_MCRF4XX  = 0x1021;
    constexpr static u16 CRC16_RIELLO   = 0x1021;
    constexpr static u16 CRC16_T10_DIF  = 0x8BB7;
    constexpr static u16 CRC16_TELEDISK = 0xA097;
    constexpr static u16 CRC16_USB      = 0x8005;
    constexpr static u16 CRC16_X25      = 0x1021;
    constexpr static u16 CRC16_XMODEM   = 0x1021;
    constexpr static u16 CRC16_MODBUS   = 0x8005;
    constexpr static u16 CRC16_KERMIT   = 0x1189;
    constexpr static u16 CRC16_TMS37157 = 0x1021;
    constexpr static u16 CRC16_A        = 0x1021;
};

}  // namespace wibot

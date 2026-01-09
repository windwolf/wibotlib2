#pragma once

//
// Created by zhouj on 2023/2/21.
//

#include "type.hpp"
#include "Validator.hpp"

namespace wibot {

struct Crc8Config {
    u8   poly;
    u8   init;
    u8   xorout;
    bool ref_in;
    bool ref_out;
};

class Crc8Validator : Validator<u8> {
   public:
    Crc8Validator(u8 poly, u8 init = 0x00, u8 xorout = 0x00, bool ref_in = false,
                  bool ref_out = false)
        : _cfg{poly, init, xorout, ref_in, ref_out} {};
    explicit Crc8Validator(const Crc8Config& cfg) : _cfg(cfg) {
    }
    void reset() override;
    void calculate(u8* data, u32 length) override;
    bool validate(u8* sum) override;
    u8   get();

   private:
    Crc8Config _cfg;
    u8         _crc;

   public:
    // Preset parameters as constexpr configs
    // Values aligned with RevEng CRC catalogue
    // Note: Some variants use reflected representation via ref_in/ref_out=true
    constexpr static Crc8Config DVB_S2{0xD5, 0x00, 0x00, false, false};
    constexpr static Crc8Config AUTOSAR{0x2F, 0xFF, 0xFF, false, false};
    constexpr static Crc8Config BLUETOOTH{0xA7, 0x00, 0x00, true, true};
    constexpr static Crc8Config CCITT{0x07, 0x00, 0x00, false, false};
    // Dallas/Maxim commonly used for 1-Wire; reflected poly 0x8C corresponds to 0x31 with refin/refout true
    constexpr static Crc8Config DALLAS_MAXIM{0x31, 0x00, 0x00, true, true};
    constexpr static Crc8Config DARC{0x39, 0x00, 0x00, true, true};
    constexpr static Crc8Config GSM_B{0x49, 0x00, 0xFF, false, false};
    constexpr static Crc8Config SAE_J1850{0x1D, 0xFF, 0xFF, false, false};
    constexpr static Crc8Config WCDMA{0x9B, 0x00, 0x00, true, true};
    constexpr static Crc8Config GSM_A{0x37, 0x00, 0x00, false, false};

    // Convenience factories
    static inline Crc8Validator From(const Crc8Config& cfg) {
        return Crc8Validator(cfg);
    }
    static inline Crc8Validator DvbS2() {
        return Crc8Validator(DVB_S2);
    }
    static inline Crc8Validator Autosar() {
        return Crc8Validator(AUTOSAR);
    }
    static inline Crc8Validator Bluetooth() {
        return Crc8Validator(BLUETOOTH);
    }
    static inline Crc8Validator Ccitt() {
        return Crc8Validator(CCITT);
    }
    static inline Crc8Validator DallasMaxim() {
        return Crc8Validator(DALLAS_MAXIM);
    }
    static inline Crc8Validator Darc() {
        return Crc8Validator(DARC);
    }
    static inline Crc8Validator GsmB() {
        return Crc8Validator(GSM_B);
    }
    static inline Crc8Validator SaeJ1850() {
        return Crc8Validator(SAE_J1850);
    }
    static inline Crc8Validator Wcdma() {
        return Crc8Validator(WCDMA);
    }
    static inline Crc8Validator GsmA() {
        return Crc8Validator(GSM_A);
    }
};

}  // namespace wibot


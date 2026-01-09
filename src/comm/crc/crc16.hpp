#pragma once

//
// Created by AI Assistant on 2025/7/11.
//

#include "type.hpp"
#include "Validator.hpp"

namespace wibot::comm {

struct Crc16Config {
    u16  poly;
    u16  init;
    u16  xorout;
    bool ref_in;
    bool ref_out;
};

class Crc16Validator : Validator<u8> {
   public:
    Crc16Validator(u16 poly, u16 init = 0x0000, u16 xorout = 0x0000, bool ref_in = false,
                   bool ref_out = false)
        : _cfg{poly, init, xorout, ref_in, ref_out} {};
    // Config-based constructor
    explicit Crc16Validator(const Crc16Config& cfg) : _cfg(cfg) {
    }
    void reset() override;
    void calculate(u8* data, u32 length) override;
    bool validate(u8* sum) override;
    u16  get();

   private:
    Crc16Config _cfg;
    u16         _crc;

   public:
    // Preset parameters as constexpr configs for common CRC-16 variants
    // Reference: reveng CRC catalogue
    // MODBUS: poly=0x8005, init=0xFFFF, xorout=0x0000, refin=true, refout=true
    constexpr static Crc16Config MODBUS{0x8005, 0xFFFF, 0x0000, true, true};

    // X25 (CRC-16/IBM-SDLC): poly=0x1021, init=0xFFFF, xorout=0xFFFF, refin=true, refout=true
    constexpr static Crc16Config X25{0x1021, 0xFFFF, 0xFFFF, true, true};

    // CCITT-FALSE: poly=0x1021, init=0xFFFF, xorout=0x0000, refin=false, refout=false
    constexpr static Crc16Config CCITT_FALSE{0x1021, 0xFFFF, 0x0000, false, false};

    // XMODEM: poly=0x1021, init=0x0000, xorout=0x0000, refin=false, refout=false
    constexpr static Crc16Config XMODEM{0x1021, 0x0000, 0x0000, false, false};

    // KERMIT: poly=0x1021 (reflected 0x8408), init=0x0000, xorout=0x0000, refin=true, refout=true
    // KERMIT uses reflected representation of 0x1021; with refin/refout true, 0x1021 works
    constexpr static Crc16Config KERMIT{0x1021, 0x0000, 0x0000, true, true};

    // DNP: poly=0x3D65, init=0x0000, xorout=0xFFFF, refin=true, refout=true
    constexpr static Crc16Config DNP{0x3D65, 0x0000, 0xFFFF, true, true};

    // MAXIM: poly=0x8005, init=0x0000, xorout=0xFFFF, refin=true, refout=true
    constexpr static Crc16Config MAXIM{0x8005, 0x0000, 0xFFFF, true, true};

    // Convenience factories to avoid misconfiguration
    static inline Crc16Validator From(const Crc16Config& cfg) {
        return Crc16Validator(cfg);
    }
    static inline Crc16Validator Modbus() {
        return Crc16Validator(MODBUS);
    }
    static inline Crc16Validator X25Validator() {
        return Crc16Validator(X25);
    }
    static inline Crc16Validator CcittFalse() {
        return Crc16Validator(CCITT_FALSE);
    }
    static inline Crc16Validator Xmodem() {
        return Crc16Validator(XMODEM);
    }
    static inline Crc16Validator Kermit() {
        return Crc16Validator(KERMIT);
    }
    static inline Crc16Validator Dnp() {
        return Crc16Validator(DNP);
    }
    static inline Crc16Validator Maxim() {
        return Crc16Validator(MAXIM);
    }
};

}  // namespace wibot

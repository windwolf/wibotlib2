#pragma once

#include "buffer.hpp"
#include "peripheral.hpp"
#include "wait-handler.hpp"

namespace wibot {

DAC_PER_DECL

using DacChannel = u8;

constexpr DacChannel kDacChannel1 = 0x01;
constexpr DacChannel kDacChannel2 = 0x02;

using DacAlignment                     = u8;
constexpr DacAlignment kDacAlignment8  = 0x08;
constexpr DacAlignment kDacAlignment12 = 0x00;

union DacConfig {
    struct {
        DacAlignment alignment;
    };
    u32 value;
};

class Dac : private PeripheralBase, private Initializable {
   public:
    Dac(DAC_CTOR_ARG);
    ~Dac();

    Result setConfig(DacConfig& config);

    Result setValue(DacChannel channel, f32 value);
    Result start(DacChannel channel);
    Result stop(DacChannel channel);

    Result calibrate(DacChannel channel);

   private:
    void _init() override;

   private:
    DAC_FIELD_DECL
    DacConfig _config;
};

};  // namespace wibot

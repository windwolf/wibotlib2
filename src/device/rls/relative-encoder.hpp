#pragma once

#include "dsp/filter/lowpass.hpp"

namespace wibot {

struct RelativeEncoderConfig {
    u32 wrapRange;
    f32 maxSpeed;
    f32 sampleTime;
    // f32    lpFilterFc;
};

class RelativeEncoder {
   public:
    void updatePositionValue(u32 value);

    void reset(f32 position, f32 speed);

    void setConfig(RelativeEncoderConfig& config);

    f32 getPosition() const {
        return position_;
    };
    f32 getSpeed() const {
        return speed_;
    };

   private:
    RelativeEncoderConfig _config;

    u32 lastValue_;

    f32 position_ = 0;
    f32 speed_    = 0;

    Lowpass filteredSpeed_;
};

}  // namespace wibot

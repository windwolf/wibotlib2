#pragma once

#include "../pipeline.hpp"
#include "dsp/util/offset-calibrator.hpp"

namespace wibot {

class OffsetCalibratorNode : public INode {
   public:
    struct Inputs {
        In<u16>  sample;
        In<bool> enableSample;
        In<bool> requestCalculate;
        In<bool> requestReset;
    } inputs;

    struct Outputs {
        Out<i16>  offset;
        Out<u16>  sampleCount;
        Out<bool> calculated;
    } outputs;

    OffsetCalibratorNode() = default;

    bool ready() override;

    void process() override;

    void reset() override;

   private:
    OffsetCalibrator _calibrator;
};

}  // namespace wibot

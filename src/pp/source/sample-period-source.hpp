#pragma once

#include "../pipeline.hpp"
#include "hal/system.hpp"

namespace wibot {

class SamplePeriodSource : public INode {
   public:
    struct Outputs {
        Out<f32> samplePeriod;    // 采样周期（秒）
        Out<u32> samplePeriodMs;  // 系统时间戳（毫秒）
    } outputs;

    /**
         * @brief 构造函数
         */
    SamplePeriodSource() {
    }

    bool ready() override {
        return outputs.samplePeriod.bound() && outputs.samplePeriodMs.bound();
    }

    void process() override {
        u32 period                   = _core.getSamplePeriodMs();
        outputs.samplePeriodMs.ref() = period;
        outputs.samplePeriod.ref()   = static_cast<f32>(period) / 1000.0f;
    }

    void reset() override {
        _core.reset();
    }

   private:
    SystemTick _core;
};
}  // namespace wibot
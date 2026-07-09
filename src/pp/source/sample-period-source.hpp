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
        return true;
    }

    void process() override {
        if (!outputs.samplePeriod.bound() && !outputs.samplePeriodMs.bound()) {
            return;
        }

        u32 period = _core.getSamplePeriodMs();
        if (outputs.samplePeriodMs.bound()) {
            outputs.samplePeriodMs.ref() = period;
        }
        if (outputs.samplePeriod.bound()) {
            outputs.samplePeriod.ref() = static_cast<f32>(period) / 1000.0f;
        }
    }

    void reset() override {
        _core.reset();
    }

   private:
    SystemTick _core;
};
}  // namespace wibot

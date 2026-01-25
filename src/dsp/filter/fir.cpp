#include "dsp/filter/fir.hpp"
#include "math.hpp"

namespace wibot {

FIR::FIR(const Config& cfg) : _config(cfg) {
    reset();
    applyConfig();
}

bool FIR::applyConfig() {
    if (!isConfigValid(_config)) {
        return false;
    }
    _tapCount = _config.tapCount;

    // 复制外部提供的系数
    for (u16 i = 0; i < _tapCount; i++) {
        _coeffs[i] = _config.coeffs[i];
    }

#if defined(CMSIS_DSP_ENABLED)
    // 初始化并清零CMSIS-DSP状态缓冲区
    for (u16 i = 0; i < _tapCount; i++) {
        _firState[i] = 0.0f;
    }
    // 初始化CMSIS-DSP FIR实例 (blockSize=1，因为我们每次处理一个样本)
    arm_fir_init_f32(&_firInstance, _tapCount, _coeffs, _firState, 1);
#else
    // 清零自定义样本缓冲区
    for (u16 i = 0; i < _tapCount; i++) {
        _xbuf[i] = 0.0f;
    }
    _idx = 0;
#endif

    return true;
}

#if defined(CMSIS_DSP_ENABLED)
f32 FIR::filter(f32 input) {
    f32 output = 0.0f;
    arm_fir_f32(&_firInstance, &input, &output, 1);
    return output;
}
#else
f32 FIR::filter(f32 input) {
    // 自定义FIR实现
    _xbuf[_idx] = input;

    f32 y = 0.0f;
    for (u16 i = 0; i < _tapCount; i++) {
        u16 pos = (_idx + _tapCount - i) % _tapCount;
        y += _coeffs[i] * _xbuf[pos];
    }

    _idx = (_idx + 1) % _tapCount;
    return y;
}
#endif

void FIR::reset() {
    for (u16 i = 0; i < MAX_TAPS; i++) {
        _coeffs[i] = 0.0f;
    }

#if defined(CMSIS_DSP_ENABLED)
    // 清零CMSIS-DSP状态缓冲区
    for (u16 i = 0; i < _tapCount; i++) {
        _firState[i] = 0.0f;
    }
#else
    // 清零自定义样本缓冲区
    for (u16 i = 0; i < _tapCount; i++) {
        _xbuf[i] = 0.0f;
    }
    _idx = 0;
#endif
}

bool FIR::isConfigValid(const Config& cfg) {
    if (cfg.coeffs == nullptr) {
        return false;
    }
    if (cfg.tapCount == 0 || cfg.tapCount > MAX_TAPS) {
        return false;
    }
    return true;
}

}  // namespace wibot

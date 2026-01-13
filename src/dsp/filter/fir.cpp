#include "dsp/filter/fir.hpp"
#include "math.hpp"
#include <cmath>

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
    _designCoeffs(_config.samplePeriod);
    _lastDesignPeriod = _config.samplePeriod;
    return true;
}

f32 FIR::filter(f32 input) {
    // 推入新样本
    if (_first) {
        for (u16 i = 0; i < _tapCount; i++) {
            _xbuf[i] = input;
        }
        _idx   = 0;
        _first = false;
    } else {
        _idx        = (_idx + 1) % _tapCount;
        _xbuf[_idx] = input;
    }

    if (_config.wrapValue <= 0.0f) {
        f32 y = 0.0f;
        for (u16 i = 0; i < _tapCount; i++) {
            u16 pos = (u16)((_idx + _tapCount - i) % _tapCount);
            y += _coeffs[i] * _xbuf[pos];
        }
        return y;
    } else {
        const f32 base = input;
        f32       y    = base;  // 系数归一化至 1，对周期数据用差分叠加
        for (u16 i = 0; i < _tapCount; i++) {
            u16 pos   = (u16)((_idx + _tapCount - i) % _tapCount);
            f32 diff  = _xbuf[pos] - base;
            f32 wdiff = _wrap(diff, _config.wrapValue);
            y += _coeffs[i] * wdiff;
        }
        return _wrap(y, _config.wrapValue);
    }
}

f32 FIR::filter(f32 input, f32 samplePeriod) {
    // 惰性更新：仅在周期变化超过容忍度时重算系数
    if (_config.periodTolerance > 0.0f && _lastDesignPeriod > 0.0f) {
        const f32 relativeChange = std::fabs(samplePeriod - _lastDesignPeriod) / _lastDesignPeriod;
        if (relativeChange > _config.periodTolerance) {
            _designCoeffs(samplePeriod);
            _lastDesignPeriod = samplePeriod;
        }
    } else {
        // periodTolerance == 0 或首次调用：每次都重算（兼容原行为）
        _designCoeffs(samplePeriod);
        _lastDesignPeriod = samplePeriod;
    }
    return filter(input);
}

void FIR::reset() {
    for (u16 i = 0; i < MAX_TAPS; i++) {
        _xbuf[i]   = 0.0f;
        _coeffs[i] = 0.0f;
    }
    _idx              = 0;
    _first            = true;
    _lastDesignPeriod = 0.0f;
}

bool FIR::isConfigValid(const Config& cfg) {
    if (cfg.samplePeriod <= 0.0f || cfg.cutoffFreq <= 0.0f) {
        return false;
    }
    if (cfg.tapCount == 0 || cfg.tapCount > MAX_TAPS) {
        return false;
    }
    // 建议奇数抽头以获得对称线性相位中心
    if ((cfg.tapCount % 2u) == 0u) {
        return false;
    }
    const f32 fs = 1.0f / cfg.samplePeriod;
    if (cfg.cutoffFreq >= fs * 0.5f) {
        return false;
    }
    if (cfg.wrapValue < 0.0f) {
        return false;
    }
    return true;
}

void FIR::_designCoeffs(f32 samplePeriod) {
    const u16 N  = _tapCount;
    const u16 M  = (u16)(N - 1);
    const f32 fs = 1.0f / samplePeriod;
    const f32 fc = _config.cutoffFreq / fs;  // 归一化频率 [0, 0.5)

    // 窗函数加权的理想低通（Hamming）
    f32 sum = 0.0f;
    for (u16 n = 0; n < N; n++) {
        const f32 m = (f32)n - (f32)M * 0.5f;
        f32       h;
        if (std::fabs(m) < 1e-7f) {
            h = 2.0f * fc;
        } else {
            h = 2.0f * fc * _sinc(2.0f * fc * m);
        }
        const f32 w = 0.54f - 0.46f * std::cos(2.0f * kPI * (f32)n / (f32)M);
        const f32 c = h * w;
        _coeffs[n]  = c;
        sum += c;
    }

    // 归一化到单位增益
    if (sum != 0.0f) {
        const f32 inv = 1.0f / sum;
        for (u16 n = 0; n < N; n++) {
            _coeffs[n] *= inv;
        }
    }
}

f32 FIR::_sinc(f32 x) {
    // sinc(x) = sin(pi*x)/(pi*x)
    const f32 pix = kPI * x;
    if (std::fabs(pix) < 1e-7f) {
        return 1.0f;
    }
    return std::sin(pix) / pix;
}

f32 FIR::_wrap(f32 x, f32 w) {
    return x - 2.0f * w * std::floor((x + w) / (2.0f * w));
}

}  // namespace wibot

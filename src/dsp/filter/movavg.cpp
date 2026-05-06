#include "dsp/filter/movavg.hpp"
#include <cmath>

namespace wibot {

MovingAverage::MovingAverage(const Config& cfg) : _config(cfg) {
    reset();
    applyConfig();
}

bool MovingAverage::applyConfig() {
    ASSERT(isConfigValid(_config), "Invalid MovingAverage config");
    _designWindow(_config.samplePeriod);
    _lastDesignPeriod = _config.samplePeriod;
    return true;
}

f32 MovingAverage::filter(f32 input) {
    // 推入新样本
    if (_first) {
        for (u16 i = 0; i < _tapCount; i++) {
            _xbuf[i] = input;
        }
        _idx   = 0;
        _sum   = input * (f32)_tapCount;
        _first = false;
    } else {
        _idx        = (u16)((_idx + 1) % _tapCount);
        const f32 o = _xbuf[_idx];
        _xbuf[_idx] = input;
        if (_config.wrapValue <= 0.0f) {
            _sum += input - o;
        } else {
            // 周期数据：运行和不适用，按差分折叠求均值（O(N)）
        }
    }

    if (_config.wrapValue <= 0.0f) {
        return _sum * _invTap;
    } else {
        const f32 base = input;
        f32       acc  = 0.0f;
        for (u16 i = 0; i < _tapCount; i++) {
            u16       pos   = (u16)((_idx + _tapCount - i) % _tapCount);
            const f32 diff  = _xbuf[pos] - base;
            const f32 wdiff = _wrap(diff, _config.wrapValue);
            acc += wdiff;
        }
        const f32 y = base + acc * _invTap;
        return _wrap(y, _config.wrapValue);
    }
}

f32 MovingAverage::filter(f32 input, f32 samplePeriod) {
    // 惰性更新：仅在周期变化超过容忍度时重算窗口
    if (_config.periodTolerance > 0.0f && _lastDesignPeriod > 0.0f) {
        const f32 rc = std::fabs(samplePeriod - _lastDesignPeriod) / _lastDesignPeriod;
        if (rc > _config.periodTolerance) {
            _designWindow(samplePeriod);
            _lastDesignPeriod = samplePeriod;
        }
    } else {
        _designWindow(samplePeriod);
        _lastDesignPeriod = samplePeriod;
    }
    return filter(input);
}

void MovingAverage::reset() {
    for (u16 i = 0; i < MAX_TAPS; i++) {
        _xbuf[i] = 0.0f;
    }
    _idx              = 0;
    _sum              = 0.0f;
    _invTap           = 1.0f;
    _tapCount         = 1;
    _lastDesignPeriod = 0.0f;
    _first            = true;
}

bool MovingAverage::isConfigValid(const Config& cfg) {
    if (cfg.samplePeriod <= 0.0f || cfg.cutoffFreq <= 0.0f) {
        return false;
    }
    if (cfg.wrapValue < 0.0f) {
        return false;
    }
    // 预估窗口大小并检查范围
    const f32 fs     = 1.0f / cfg.samplePeriod;
    const f32 target = 0.443f * fs / cfg.cutoffFreq;  // 近似 -3dB 截止到 boxcar 长度关系
    const u16 N      = (u16)std::round(target);
    if (N == 0 || N > MAX_TAPS) {
        return false;
    }
    return true;
}

u16 MovingAverage::_computeTapCount(f32 samplePeriod) const {
    const f32 fs     = 1.0f / samplePeriod;
    const f32 target = 0.443f * fs / _config.cutoffFreq;
    u16       N      = (u16)std::round(target);
    if (N < 1) N = 1;
    if (N > MAX_TAPS) N = MAX_TAPS;
    return N;
}

void MovingAverage::_designWindow(f32 samplePeriod) {
    _tapCount = _computeTapCount(samplePeriod);
    _invTap   = 1.0f / (f32)_tapCount;
    // 更新运行和以匹配新的窗口大小（简化：若窗口改变，重新初始化）
    _sum      = 0.0f;
    for (u16 i = 0; i < _tapCount; i++) {
        _sum += _xbuf[i];
    }
}

f32 MovingAverage::_wrap(f32 x, f32 w) {
    return x - 2.0f * w * std::floor((x + w) / (2.0f * w));
}

}  // namespace wibot

#include "binning-mapper.hpp"

namespace wibot {

template <u8 CHANNELS>
BinningMapper<CHANNELS>::BinningMapper(SyncPipeline<f32>& upstream, const Config& config)
    : _upstream(upstream), _config(config) {
    for (u8 i = 0; i < CHANNELS; ++i) {
        _lastBinIndex[i] = INVALID_BIN_INDEX;
    }
}

template <u8 CHANNELS>
BinningMapper<CHANNELS>::~BinningMapper() {
    // 无需特殊清理
}

template <u8 CHANNELS>
u32 BinningMapper<CHANNELS>::getValue(u8 channel) const {
    if (channel >= CHANNELS) {
        return INVALID_BIN_INDEX;
    }

    f32 input = _upstream.getValue(channel);

    if (_config.enableHysteresis) {
        return _mapToBinWithHysteresis(input, channel);
    } else {
        return _mapToBinSimple(input);
    }
}

template <u8 CHANNELS>
void BinningMapper<CHANNELS>::reset() {
    for (u8 i = 0; i < CHANNELS; ++i) {
        _lastBinIndex[i] = INVALID_BIN_INDEX;
    }
    _upstream.reset();
}

template <u8 CHANNELS>
void BinningMapper<CHANNELS>::update() {
    _upstream.update();
}

template <u8 CHANNELS>
void BinningMapper<CHANNELS>::updateConfig(const Config& config) {
    _config = config;
    for (u8 i = 0; i < CHANNELS; ++i) {
        _lastBinIndex[i] = INVALID_BIN_INDEX;
    }
}

template <u8 CHANNELS>
u32 BinningMapper<CHANNELS>::_mapToBinSimple(f32 value) const {
    if (_config.ranges == nullptr || _config.binCount == 0) {
        return _config.clampToRange ? 0 : INVALID_BIN_INDEX;
    }

    // 查找值所属的区间
    for (u32 i = 0; i < _config.binCount; ++i) {
        if (value >= _config.ranges[i].lowerBound && value < _config.ranges[i].upperBound) {
            return i;
        }
    }

    // 处理超出范围的情况
    if (value < _config.ranges[0].lowerBound) {
        return _config.clampToRange ? 0 : INVALID_BIN_INDEX;
    } else {
        return _config.clampToRange ? (_config.binCount - 1) : INVALID_BIN_INDEX;
    }
}

template <u8 CHANNELS>
u32 BinningMapper<CHANNELS>::_mapToBinWithHysteresis(f32 value, u8 channel) const {
    u32 currentBin = _mapToBinSimple(value);

    if (currentBin == INVALID_BIN_INDEX) {
        return INVALID_BIN_INDEX;
    }

    u32 lastBin = _lastBinIndex[channel];
    // 第一次调用，直接返回当前区间
    if (lastBin == INVALID_BIN_INDEX) {
        _lastBinIndex[channel] = currentBin;
        return currentBin;
    }

    // 如果区间未变化，直接返回
    if (currentBin == lastBin) {
        return currentBin;
    }

    // 检查是否需要滞回处理（只处理相邻区间）

    bool isAdjacent = (currentBin == lastBin + 1) || (currentBin + 1 == lastBin);

    if (isAdjacent && _config.hysteresisWidth > 0.0f) {
        u32 boundaryIndex = (currentBin < lastBin) ? currentBin : lastBin;

        if (boundaryIndex < _config.binCount - 1) {
            // 计算边界的滞回区间
            f32 boundary  = _config.ranges[boundaryIndex].upperBound;
            f32 halfWidth = _config.hysteresisWidth * 0.5f;

            // 如果值在滞回区间内，保持上次结果
            if (value >= (boundary - halfWidth) && value <= (boundary + halfWidth)) {
                return _lastBinIndex[channel];
            }
        }
    }

    // 更新并返回新区间
    _lastBinIndex[channel] = currentBin;
    return currentBin;
}

}  // namespace wibot

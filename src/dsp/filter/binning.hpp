#pragma once

#include "type.hpp"
#include <type_traits>
#include <concepts>

namespace wibot {

template <typename T>
    requires SupportArithmetic<T>
class Binning {
   public:
    static constexpr u32 INVALID_BIN_INDEX = UINT32_MAX;

    struct Config {
        const T* boundaries{nullptr};      // 边界点数组，长度为 binCount - 1
        u32      binCount{0};              // 分桶数量
        T        hysteresisWidth{};        // 滞回区域宽度
        bool     enableHysteresis{false};  // 是否启用滞回功能
    };

    struct State {
        u32 currentBin{UINT32_MAX};  // 当前分桶索引
    };

    explicit Binning(Config& config) : _config(config) {
    }

    void reset() {
        _state.currentBin = INVALID_BIN_INDEX;
    }

    u32 process(T input) {
        if (_config.enableHysteresis) {
            _state.currentBin = mapToBinWithHysteresis(input);
        } else {
            _state.currentBin = mapToBinSimple(input);
        }
        return _state.currentBin;
    }

   private:
    u32 mapToBinSimple(T value) const {
        if (_config.binCount == 0 || _config.boundaries == nullptr) {
            return INVALID_BIN_INDEX;
        }

        if (_config.binCount == 1) {
            return 0;
        }

        // 查找值所属的分桶
        for (u32 i = 0; i < _config.binCount - 1; ++i) {
            if (value < _config.boundaries[i]) {
                return i;
            }
        }

        return _config.binCount - 1;
    }

    u32 mapToBinWithHysteresis(T value) {
        u32 currentBin = mapToBinSimple(value);

        if (currentBin == INVALID_BIN_INDEX || _state.currentBin == INVALID_BIN_INDEX) {
            return currentBin;
        }

        if (currentBin == _state.currentBin) {
            return currentBin;
        }

        // 检查是否需要滞回处理（只处理相邻区间）
        bool isAdjacent =
            (currentBin == _state.currentBin + 1) || (currentBin + 1 == _state.currentBin);

        if (isAdjacent && _config.hysteresisWidth > 0) {
            u32  boundaryIndex = (currentBin < _state.currentBin) ? currentBin : _state.currentBin;
            auto boundary      = _config.boundaries[boundaryIndex];
            auto halfWidth     = _config.hysteresisWidth * static_cast<T>(0.5);

            // 如果值在滞回区间内，保持上次结果
            if (value >= (boundary - halfWidth) && value <= (boundary + halfWidth)) {
                return _state.currentBin;
            }
        }

        return currentBin;
    }

   private:
    Config&      _config;
    State _state{};
};

} // namespace wibot


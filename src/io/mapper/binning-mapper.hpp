#pragma once

#include "model.hpp"
#include <array>
#include <type_traits>

namespace wibot {

/**
 * @brief 分桶映射管道
 * 
 * 将输入值映射到离散的桶索引，支持滞回功能避免抖动。
 * 配置使用引用方式，支持多个映射器实例共享同一配置。
 * 
 * @tparam T 输入数据类型
 */
template <typename T>
class BinningMapper : public SyncPipeline<u32> {
   public:
    /**
     * @brief 分桶映射配置
     * 
     * 边界点顺序：boundaries[0] < boundaries[1] < ... < boundaries[binCount-2]
     * 分桶规则：
     *   value < boundaries[0]                    → 分桶 0
     *   boundaries[0] <= value < boundaries[1]   → 分桶 1
     *   boundaries[1] <= value < boundaries[2]   → 分桶 2
     *   ...
     *   value >= boundaries[binCount-2]         → 分桶 binCount-1
     */
    struct Config {
        const T* boundaries;        ///< 边界点数组，长度为 binCount - 1，按升序排列
        u32      binCount;          ///< 分桶数量
        T        hysteresisWidth;   ///< 滞回区域宽度
        bool     enableHysteresis;  ///< 是否启用滞回功能
    };

    /**
     * @brief 无效区间索引标记
     */
    static constexpr u32 INVALID_BIN_INDEX = UINT32_MAX;

   public:
    /**
     * @brief 构造分桶映射器
     * 
     * @param upstream 上游管道
     * @param config 分桶配置（引用方式，支持共享）
     */
    BinningMapper(SyncPipeline<T>& upstream, const Config& config)
        : _upstream(upstream), _config(config), _currentValue(INVALID_BIN_INDEX) {
    }

    ~BinningMapper() = default;

    u32 getValue() const override {
        return _currentValue;
    }

    void reset() override {
        _currentValue = INVALID_BIN_INDEX;
        _upstream.reset();
    }

    void update() override {
        _upstream.update();
        T input = _upstream.getValue();

        if (_config.enableHysteresis) {
            _currentValue = mapToBinWithHysteresis(input);
        } else {
            _currentValue = mapToBinSimple(input);
        }
    }

   private:
    /**
     * @brief 执行分桶映射（不带滞回）
     */
    u32 mapToBinSimple(T value) const {
        if (_config.binCount == 0 || _config.boundaries == nullptr) {
            return INVALID_BIN_INDEX;
        }

        if (_config.binCount == 1) {
            return 0;
        }

        // 查找值所属的分桶 - 找到第一个大于value的边界点
        for (u32 i = 0; i < _config.binCount - 1; ++i) {
            if (value < _config.boundaries[i]) {
                return i;
            }
        }

        return _config.binCount - 1;
    }

    /**
     * @brief 执行分桶映射（带滞回）
     */
    u32 mapToBinWithHysteresis(T value) {
        u32 currentBin = mapToBinSimple(value);

        if (currentBin == INVALID_BIN_INDEX || _currentValue == INVALID_BIN_INDEX) {
            return currentBin;
        }

        if (currentBin == _currentValue) {
            return currentBin;
        }

        // 检查是否需要滞回处理（只处理相邻区间）
        bool isAdjacent = (currentBin == _currentValue + 1) || (currentBin + 1 == _currentValue);

        if (isAdjacent && _config.hysteresisWidth > 0) {
            u32  boundaryIndex = (currentBin < _currentValue) ? currentBin : _currentValue;
            auto boundary      = _config.boundaries[boundaryIndex];
            auto halfWidth     = _config.hysteresisWidth * static_cast<T>(0.5);

            // 如果值在滞回区间内，保持上次结果
            if (value >= (boundary - halfWidth) && value <= (boundary + halfWidth)) {
                return _currentValue;
            }
        }

        return currentBin;
    }

   private:
    SyncPipeline<T>& _upstream;      ///< 上游管道引用
    const Config&    _config;        ///< 分桶配置引用（支持共享）
    u32              _currentValue;  ///< 当前分桶值
};

}  // namespace wibot

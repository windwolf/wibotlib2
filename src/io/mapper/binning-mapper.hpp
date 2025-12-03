#pragma once

#include "model.hpp"
#include <array>
#include <type_traits>

namespace wibot {

/**
 * @brief 分桶映射核心算法类（类型擦除，减少代码膨胀）
 * 
 * 将与通道数量无关的核心逻辑提取出来，避免每个CHANNELS值都生成一份代码
 * 使用边界点数组简化配置，利用相邻分桶边界重合的特点
 */
template <typename T>
class BinningMapperCore {
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
        const T*   boundaries;        ///< 边界点数组，长度为 binCount - 1，按升序排列
        u32  binCount;          ///< 分桶数量
        T    hysteresisWidth;   ///< 滞回区域宽度
        bool enableHysteresis;  ///< 是否启用滞回功能
    };

    /**
     * @brief 无效区间索引标记
     */
    static constexpr u32 INVALID_BIN_INDEX = UINT32_MAX;

    /**
     * @brief 执行分桶映射（不带滞回）
     */
    static u32 mapToBinSimple(T value, const Config& config) {
        if (config.binCount == 0) {
            return INVALID_BIN_INDEX;
        }

        // 只有一个分桶的情况
        if (config.binCount == 1) {
            return 0;
        }

        if (config.boundaries == nullptr) {
            return INVALID_BIN_INDEX;
        }

        // 查找值所属的分桶
        // 遍历所有边界点，找到第一个大于value的边界点
        for (u32 i = 0; i < config.binCount - 1; ++i) {
            if (value < config.boundaries[i]) {
                return i;  // 归为分桶i
            }
        }

        // 值大于等于最后一个边界点，归为最后一个分桶
        return config.binCount - 1;
    }

    /**
     * @brief 执行分桶映射（带滞回）
     */
    static u32 mapToBinWithHysteresis(T value, u32 lastBin, const Config& config) {
        u32 currentBin = mapToBinSimple(value, config);

        if (currentBin == INVALID_BIN_INDEX) {
            return INVALID_BIN_INDEX;
        }

        // 第一次调用，直接返回当前区间
        if (lastBin == INVALID_BIN_INDEX) {
            return currentBin;
        }

        // 如果区间未变化，直接返回
        if (currentBin == lastBin) {
            return currentBin;
        }

        // 检查是否需要滞回处理（只处理相邻区间）
        bool isAdjacent = (currentBin == lastBin + 1) || (currentBin + 1 == lastBin);

        if (isAdjacent && config.hysteresisWidth > 0) {
            // 计算相邻分桶之间的边界索引
            // 对于分桶i和分桶i+1之间的边界，边界索引为i
            u32 boundaryIndex = (currentBin < lastBin) ? currentBin : (lastBin);

            if (boundaryIndex < config.binCount - 1) {
                // 计算边界的滞回区间
                auto boundary  = config.boundaries[boundaryIndex];
                auto halfWidth = config.hysteresisWidth * static_cast<T>(0.5);

                // 如果值在滞回区间内，保持上次结果
                if (value >= (boundary - halfWidth) && value <= (boundary + halfWidth)) {
                    return lastBin;
                }
            }
        }

        // 返回新区间
        return currentBin;
    }
};

/**
 * @brief 通道数组操作助手（模板元编程优化）
 */
template <u8 CHANNELS>
struct ChannelArrayHelper {
    template <typename F>
    static void forEachChannel(F&& func) {
        if constexpr (CHANNELS == 1) {
            func(0);
        } else {
            for (u8 i = 0; i < CHANNELS; ++i) {
                func(i);
            }
        }
    }

    static void resetArray(u32* array, u32 value) {
        if constexpr (CHANNELS == 1) {
            array[0] = value;
        } else {
            for (u8 i = 0; i < CHANNELS; ++i) {
                array[i] = value;
            }
        }
    }
};

/**
 * @brief 分桶映射管道（优化版）
 * 
 * 使用模板元编程减少代码膨胀，将核心算法提取到BinningMapperCore中。
 * 
 * @tparam T 输入数据类型
 * @tparam CHANNELS 通道数量，编译时确定
 */
template <typename T, u8 CHANNELS>
class BinningMapper : public SyncPipeline<u32> {
   private:
    using Core = BinningMapperCore<T>;

   public:
    using Config                           = typename Core::Config;
    /**
     * @brief 无效区间索引标记
     */
    static constexpr u32 INVALID_BIN_INDEX = Core::INVALID_BIN_INDEX;

   public:
    /**
     * @brief 构造分桶映射器
     * 
     * @param upstream 上游管道
     * @param config 分桶配置
     */
    BinningMapper(SyncPipeline<T>& upstream, const Config& config)
        : _upstream(upstream), _config(config) {
        ChannelArrayHelper<CHANNELS>::resetArray(_currentValues.data(), INVALID_BIN_INDEX);
    }

    /**
     * @brief 析构函数，清理资源
     */
    ~BinningMapper() = default;

    /**
     * @brief 获取分桶映射后的区间索引
     */
    u32 getValue(u8 channel) const override {
        if constexpr (CHANNELS == 1) {
            return channel == 0 ? _currentValues[0] : INVALID_BIN_INDEX;
        } else {
            return channel < CHANNELS ? _currentValues[channel] : INVALID_BIN_INDEX;
        }
    }

    u32* getValues() const override {
        return const_cast<u32*>(_currentValues.data());
    }

    /**
     * @brief 重置管道状态
     */
    void reset() override {
        ChannelArrayHelper<CHANNELS>::resetArray(_currentValues.data(), INVALID_BIN_INDEX);
        _upstream.reset();
    }

    /**
     * @brief 更新管道状态
     */
    void update() override {
        _upstream.update();

        // 使用编译期优化的循环
        ChannelArrayHelper<CHANNELS>::forEachChannel([this](u8 i) {
            T input = _upstream.getValue(i);

            if (_config.enableHysteresis) {
                _currentValues[i] = Core::mapToBinWithHysteresis(input, _currentValues[i], _config);
            } else {
                _currentValues[i] = Core::mapToBinSimple(input, _config);
            }
        });
    }

    /**
     * @brief 更新分桶配置（影响所有通道）
     */
    void updateConfig(const Config& config) {
        _config = config;
        ChannelArrayHelper<CHANNELS>::resetArray(_currentValues.data(), INVALID_BIN_INDEX);
    }

    /**
     * @brief 获取通道数量
     */
    constexpr u8 getChannelCount() const {
        return CHANNELS;
    }

   private:
    SyncPipeline<T>&                  _upstream;       ///< 上游管道引用
    Config                            _config;         ///< 分桶配置
    mutable std::array<u32, CHANNELS> _currentValues;  ///< 每通道的当前分桶值
};

/**
 * @brief 单通道分桶映射器特化（进一步减少代码膨胀）
 */
template <typename T>
class BinningMapper<T, 1> : public SyncPipeline<u32> {
   private:
    using Core = BinningMapperCore<T>;

   public:
    using Config = typename Core::Config;

    static constexpr u32 INVALID_BIN_INDEX = Core::INVALID_BIN_INDEX;

   public:
    /**
     * @brief 构造单通道分桶映射器
     */
    BinningMapper(SyncPipeline<T>& upstream, const Config& config)
        : _upstream(upstream), _config(config), _currentValue(INVALID_BIN_INDEX) {
    }

    /**
     * @brief 析构函数
     */
    ~BinningMapper() = default;

    /**
     * @brief 获取分桶映射后的区间索引
     */
    u32 getValue(u8 channel) const override {
        return channel == 0 ? _currentValue : INVALID_BIN_INDEX;
    }

    u32* getValues() const override {
        return const_cast<u32*>(&_currentValue);
    }

    /**
     * @brief 重置管道状态
     */
    void reset() override {
        _currentValue = INVALID_BIN_INDEX;
        _upstream.reset();
    }

    /**
     * @brief 更新管道状态
     */
    void update() override {
        _upstream.update();
        T input = _upstream.getValue(0);

        if (_config.enableHysteresis) {
            _currentValue = Core::mapToBinWithHysteresis(input, _currentValue, _config);
        } else {
            _currentValue = Core::mapToBinSimple(input, _config);
        }
    }

    /**
     * @brief 更新分桶配置
     */
    void updateConfig(const Config& config) {
        _config       = config;
        _currentValue = INVALID_BIN_INDEX;
    }

    /**
     * @brief 获取通道数量
     */
    constexpr u8 getChannelCount() const {
        return 1;
    }

   private:
    SyncPipeline<T>& _upstream;      ///< 上游管道引用
    Config           _config;        ///< 分桶配置
    mutable u32      _currentValue;  ///< 当前分桶值
};

/**
 * @brief 便利函数：根据边界点数组创建BinningMapper配置
 * 
 * @param boundaries 边界点数组，长度为 binCount - 1，按升序排列
 * @param binCount 分桶数量
 * @param hysteresisWidth 滞回宽度
 * @param enableHysteresis 是否启用滞回
 * 
 * 示例：创建3个分桶 [<10), [10,20), [>=20)
 * float boundaries[] = {10.0f, 20.0f};  // 长度为3-1=2
 * auto config = makeBinningConfig<float>(boundaries, 3);
 * 
 * 分桶逻辑：
 *   value < 10.0f  → 分桶 0
 *   10.0f <= value < 20.0f → 分桶 1  
 *   value >= 20.0f → 分桶 2
 */
template <typename T>
typename BinningMapperCore<T>::Config makeBinningConfig(T* boundaries, u32 binCount,
                                                        T    hysteresisWidth  = T{0},
                                                        bool enableHysteresis = false) {
    typename BinningMapperCore<T>::Config config;
    config.boundaries       = boundaries;
    config.binCount         = binCount;
    config.hysteresisWidth  = hysteresisWidth;
    config.enableHysteresis = enableHysteresis;
    return config;
}

}  // namespace wibot

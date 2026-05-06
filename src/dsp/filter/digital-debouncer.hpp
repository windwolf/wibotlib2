#pragma once

#include "type.hpp"

namespace wibot {

/**
 * @brief 多通道数字输入处理核心
 * 
 * 支持反转（按位XOR）和去抖（时间窗口稳定）。
 * 
 * @tparam CHANNELS 通道数量（最多32个）
 * @tparam GetTickMsFn 获取当前时刻的函数指针，默认由使用者定义
 */
template <u8 CHANNELS>
    requires(CHANNELS <= 32)
class DigitalDebouncer {
   public:
    /**
 * @brief 数字输入反转和去抖配置
 */
    struct Config {
        u32 inverse{0};         // 反转掩码，32位对应最多32个通道
        u8  debounceTimeMs{0};  // 消抖时间（毫秒）
    };

    /**
 * @brief 数字输入状态
 */
    struct State {
        bool isFirstValue{true};
        u32  rawStatus{0};
        u32  lastOutputStatus{0};
        u32  lastBufferedStatus{0};
        u8   debounceTimers[CHANNELS]{};  // 各通道累积去抖时间（毫秒，最大255ms）
    };

   public:
    explicit DigitalDebouncer(Config& config) : _config(config) {
    }

    void reset() {
        _state.isFirstValue       = true;
        _state.lastOutputStatus   = 0;
        _state.lastBufferedStatus = 0;

        for (u8 i = 0; i < CHANNELS; ++i) {
            _state.debounceTimers[i] = 0;
        }
    }

    void updateRawValues(u32 rawValues) {
        _state.rawStatus = rawValues & channelMask();
    }

    /**
     * @brief 处理数字输入
     * 
     * @param samplePeriodMs 当前采样周期 (ms)
     * @return u32 处理后的多通道输出（位掩码）
     */
    u32 process(u32 samplePeriodMs) {
        if (_state.isFirstValue) {
            _state.isFirstValue       = false;
            _state.lastBufferedStatus = _state.rawStatus;
            _state.lastOutputStatus   = (_state.rawStatus ^ _config.inverse) & channelMask();

            if (_config.debounceTimeMs > 0) {
                for (u8 i = 0; i < CHANNELS; ++i) {
                    _state.debounceTimers[i] = 0;
                }
            }
            return _state.lastOutputStatus;
        }

        // 不启用去抖时，直接反转输出
        if (_config.debounceTimeMs == 0) {
            _state.lastOutputStatus = (_state.rawStatus ^ _config.inverse) & channelMask();
            return _state.lastOutputStatus;
        }

        // 去抖逻辑
        u32 changedChannels = (_state.rawStatus ^ _state.lastBufferedStatus) & channelMask();

        if (changedChannels != 0) {
            // 更新缓冲状态，重置变化通道的去抖计时器
            _state.lastBufferedStatus = _state.rawStatus & channelMask();
            for (u8 i = 0; i < CHANNELS; ++i) {
                if (changedChannels & (1U << i)) {
                    _state.debounceTimers[i] = 0;
                }
            }
        }

        // 检查所有通道的去抖
        for (u8 j = 0; j < CHANNELS; ++j) {
            bool bufferedBit = (_state.lastBufferedStatus >> j) & 1U;
            bool outputBit   = (_state.lastOutputStatus >> j) & 1U;

            // 应用反转掩码到缓冲位
            bool processedBufferedBit =
                ((_config.inverse >> j) & 1U) ? (!bufferedBit) : bufferedBit;

            if (processedBufferedBit != outputBit) {
                // 通道值改变，累积去抖时间
                u32 newTimerValue = _state.debounceTimers[j] + samplePeriodMs;

                // 检查去抖时间是否达到阈值
                if (newTimerValue >= _config.debounceTimeMs) {
                    // 更新输出状态位
                    if (processedBufferedBit) {
                        _state.lastOutputStatus |= (1U << j);
                    } else {
                        _state.lastOutputStatus &= ~(1U << j);
                    }
                } else {
                    _state.debounceTimers[j] = static_cast<u8>(newTimerValue);
                }
            }
        }

        _state.lastOutputStatus &= channelMask();
        return _state.lastOutputStatus;
    }

    u32 getOutput() const {
        return _state.lastOutputStatus & channelMask();
    }

    bool getChannel(u8 channel) const {
        if (channel >= CHANNELS) {
            return false;
        }
        return (_state.lastOutputStatus >> channel) & 1U;
    }

    void setConfig(Config config) {
        _config = config;
        _config.inverse &= channelMask();
    }

   private:
    static constexpr u32 channelMask() {
        if constexpr (CHANNELS >= 32) {
            return UINT32_MAX;
        } else {
            return (static_cast<u32>(1) << CHANNELS) - 1U;
        }
    }

    Config& _config{};
    State   _state;
};

}  // namespace wibot

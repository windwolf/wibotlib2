#pragma once

#include "type.hpp"

namespace wibot::dsp {

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
        u32  lastDebounceTime[CHANNELS]{};
    };

   public:
    explicit DigitalDebouncer(Config& config) : _config(config) {
    }

    void reset(u32 currentTick = 0) {
        _state.isFirstValue       = true;
        _state.lastOutputStatus   = 0;
        _state.lastBufferedStatus = 0;

        for (u8 i = 0; i < CHANNELS; ++i) {
            _state.lastDebounceTime[i] = currentTick;
        }
    }

    void updateRawValues(u32 rawValues) {
        _state.rawStatus = rawValues;
    }

    /**
     * @brief 处理数字输入
     * 
     * @param currentTick 当前时刻 (ms)
     * @return u32 处理后的多通道输出（位掩码）
     */
    u32 process(u32 currentTick) {
        if (_state.isFirstValue) {
            _state.isFirstValue       = false;
            _state.lastBufferedStatus = _state.rawStatus;
            _state.lastOutputStatus   = _state.rawStatus ^ _config.inverse;

            if (_config.debounceTimeMs > 0) {
                for (u8 i = 0; i < CHANNELS; ++i) {
                    _state.lastDebounceTime[i] = currentTick;
                }
            }
            return _state.lastOutputStatus;
        }

        // 不启用去抖时，直接反转输出
        if (_config.debounceTimeMs == 0) {
            _state.lastOutputStatus = _state.rawStatus ^ _config.inverse;
            return _state.lastOutputStatus;
        }

        // 去抖逻辑
        u32 changedChannels = _state.rawStatus ^ _state.lastBufferedStatus;

        if (changedChannels != 0) {
            // 更新缓冲状态，重置变化通道的去抖计时器
            _state.lastBufferedStatus = _state.rawStatus;
            for (u8 i = 0; i < CHANNELS; ++i) {
                if (changedChannels & (1U << i)) {
                    _state.lastDebounceTime[i] = currentTick;
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
                // 通道值改变，检查去抖时间是否超过阈值
                if (currentTick - _state.lastDebounceTime[j] > _config.debounceTimeMs) {
                    // 更新输出状态位
                    if (processedBufferedBit) {
                        _state.lastOutputStatus |= (1U << j);
                    } else {
                        _state.lastOutputStatus &= ~(1U << j);
                    }
                }
            }
        }

        return _state.lastOutputStatus;
    }

    u32 getOutput() const {
        return _state.lastOutputStatus;
    }

    bool getChannel(u8 channel) const {
        if (channel >= CHANNELS) {
            return false;
        }
        return (_state.lastOutputStatus >> channel) & 1U;
    }

    void setConfig(Config config) {
        _config = config;
    }

   private:
    Config&           _config{};
    State _state;
};

}  // namespace wibot::dsp

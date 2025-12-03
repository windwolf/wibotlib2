#pragma once

#include "model.hpp"
#include "system.hpp"
#include "type.hpp"

namespace wibot {

/**
 * @brief 数字输入源配置
 */
struct DigitalSourceConfig {
    u32 inverse;         ///< 反转掩码，32位对应最多32个通道
    u8  debounceTimeMs;  ///< 消抖时间（毫秒）
};

/**
 * @brief 多通道数字输入源
 * 
 * 支持多通道数字输入的消抖和反转处理。
 * 所有通道共享配置，适用于按键扫描等硬件多通道场景。
 * 
 * 继承 MultiChannelPipeline<bool, CHANNELS> 接口。
 * 可通过 ChannelAdapter 将特定通道适配为单通道 SyncPipeline。
 * 
 * @tparam CHANNELS 通道数量（最多32个通道）
 */
template <u8 CHANNELS>
class DigitalSource : public MultiChannelPipeline<bool, CHANNELS> {
   public:
    /**
     * @brief 构造数字输入数据源
     * 
     * @param config 数字输入配置参数
     */
    explicit DigitalSource(const DigitalSourceConfig& config)
        : _config(config), _isFirstValue(true), _lastOutputStatus(0), _lastBufferedStatus(0) {
        static_assert(CHANNELS <= 32, "CHANNELS must not exceed 32");

        // 初始化各通道消抖时间
        for (u8 i = 0; i < CHANNELS; i++) {
            _lastDebounceTime[i] = 0;
        }
    }

    /**
     * @brief 构造数字输入数据源
     * 
     * @param inverse 反转掩码，32位对应最多32个通道
     * @param debounceTimeMs 消抖时间（毫秒）
     */
    DigitalSource(u32 inverse = 0, u8 debounceTimeMs = 50)
        : _config{inverse, debounceTimeMs},
          _isFirstValue(true),
          _lastOutputStatus(0),
          _lastBufferedStatus(0) {
        static_assert(CHANNELS <= 32, "CHANNELS must not exceed 32");

        // 初始化各通道消抖时间
        for (u8 i = 0; i < CHANNELS; i++) {
            _lastDebounceTime[i] = 0;
        }
    }

    /**
     * @brief 更新所有通道数据
     */
    void update() {
        _processDigitalInput();
    }

    /**
     * @brief 获取指定通道的值
     * 
     * @param channel 通道索引
     * @return bool 通道值
     */
    bool getValue(u8 channel) const {
        if (channel >= CHANNELS) {
            return false;  // 返回无效值
        }
        return (_lastOutputStatus >> channel) & 1U;
    }

    /**
     * @brief 获取所有通道的值（位掩码格式）
     * 
     * @return u32 所有通道的状态，每位对应一个通道
     */
    u32 getValues() const {
        return _lastOutputStatus;
    }

    /**
     * @brief 重置所有通道状态
     */
    void reset() {
        _isFirstValue       = true;
        _lastOutputStatus   = 0;
        _lastBufferedStatus = 0;

        u32 currentTime = System::getTickMs();
        for (u8 i = 0; i < CHANNELS; i++) {
            _lastDebounceTime[i] = currentTime;
        }
    }

   public:
    /**
     * @brief 获取通道数量
     */
    static constexpr u8 getChannelCount() {
        return CHANNELS;
    }

    /**
     * @brief 更新所有通道的原始输入值

     * @param rawValues 原始输入值，32位掩码格式
     */
    void updateRawValues(u32 rawValues) {
        _rawStatus = rawValues;
    }

    /**
     * @brief 重新配置所有通道
     * 
     * @param config 数字输入配置参数
     */
    void configure(const DigitalSourceConfig& config) {
        _config = config;
    }

   private:
    /**
     * @brief 处理数字输入更新逻辑
     */
    void _processDigitalInput() {
        if (_isFirstValue) {
            _isFirstValue       = false;
            _lastBufferedStatus = _rawStatus;
            // Apply inverse setting per channel on first value
            _lastOutputStatus   = _rawStatus ^ _config.inverse;
            // Initialize debounce time for all channels only if debounce is enabled
            if (_config.debounceTimeMs > 0) {
                u32 currentTime = System::getTickMs();
                for (u8 i = 0; i < CHANNELS; i++) {
                    _lastDebounceTime[i] = currentTime;
                }
            }
            return;
        }

        // If debounce is disabled, apply inverse setting directly without debounce logic
        if (_config.debounceTimeMs == 0) {
            _lastOutputStatus = _rawStatus ^ _config.inverse;
            return;
        }

        // Debounce logic (only executed when debounceTimeMs > 0)
        u32 currentTime     = System::getTickMs();
        u32 changedChannels = _rawStatus ^ _lastBufferedStatus;

        if (changedChannels != 0) {
            // Update buffered status and reset debounce timer for changed channels
            _lastBufferedStatus = _rawStatus;
            for (u8 i = 0; i < CHANNELS; i++) {
                if (changedChannels & (1U << i)) {
                    _lastDebounceTime[i] = currentTime;
                }
            }
        }

        // Check debounce for all channels
        for (u8 j = 0; j < CHANNELS; j++) {
            bool bufferedBit = (_lastBufferedStatus >> j) & 1U;
            bool outputBit   = (_lastOutputStatus >> j) & 1U;

            // Apply inverse setting to buffered bit for comparison
            bool processedBufferedBit =
                ((_config.inverse >> j) & 1U) ? (!bufferedBit) : bufferedBit;

            if (processedBufferedBit != outputBit) {
                // Channel value has changed, check if debounce time has passed
                if (currentTime - _lastDebounceTime[j] > _config.debounceTimeMs) {
                    // Update the output status bit
                    if (processedBufferedBit) {
                        _lastOutputStatus |= (1U << j);
                    } else {
                        _lastOutputStatus &= ~(1U << j);
                    }
                }
            }
        }
    }

   private:
    DigitalSourceConfig _config;        ///< 配置参数
    bool                _isFirstValue;  ///< 是否首次更新
    u32                 _rawStatus;
    u32                 _lastOutputStatus;            ///< 上次输出状态（消抖后的值）
    u32                 _lastBufferedStatus;          ///< 上次缓冲状态（原始输入值）
    u32                 _lastDebounceTime[CHANNELS];  ///< 各通道上次消抖时间

   private:
    static constexpr u8 DEFAULT_DEBOUNCE_TIME_MS = 50;  ///< 默认消抖时间
};


}  // namespace wibot

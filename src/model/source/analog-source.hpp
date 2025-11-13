#pragma once

#include "model.hpp"
#include "system.hpp"
#include <cstdint>

namespace wibot {

/**
 * @brief 内存数据源
 * 
 * 用于读取由外部ADC通过DMA更新的原始值缓存
 * 
 * @tparam CHANNELS 通道数量
 */
template <u8 CHANNELS>
class AnalogSource : public SyncPipeline<i16> {
   public:
    struct Config {
        u8 adcResolution;  ///< ADC分辨率位数: 8=8位(0-255), 12=12位(0-4095), 16=16位(0-65535)
    };

    /**
     * @brief 自动校准配置
    */
    struct AutoCalibrationConfig {
        u32 sampleIntervalMs;  ///< 采样间隔时间（毫秒）
        u16 sampleCount;       ///< 采样次数
    };

   public:
    /**
     * @brief 构造内存数据源
     * 
     * @param config ADC配置参数
     * @param buffer 外部缓冲区指针，由ADC通过DMA更新的原始值数据
     */
    explicit AnalogSource(const Config& config, const u16 buffer[CHANNELS]);

    // Pipeline接口实现
    void update() override;
    i16  getValue(u8 channel) const override;
    i16* getValues() const override;
    void reset() override;

   public:
    /**
     * @brief 获取通道数量
     */
    static constexpr u8 getChannelCount() {
        return CHANNELS;
    }

    /**
     * @brief 手动设置校准偏移
     * 
     * @param offset 偏移量 (原始值)
     */
    void setCalibration(u8 channel, i16 offset);

    /**
     * @brief 获取校准偏移
     * 
     * @return i16 偏移量 (原始值)
     */
    i16 getCalibration(u8 channel) const;

    /**
     * @brief 设置外部缓冲区
     * 
     * @param buffer 外部缓冲区指针，由ADC通过DMA更新的原始值数据
     */
    void setBuffer(const u16 buffer[CHANNELS]);

    /**
     * @brief 开始自动校准
     * 
     * @param config 自动校准配置参数
     */
    void startAutoCalibration(const AutoCalibrationConfig& config);

    /**
     * @brief 停止自动校准
     */
    void stopAutoCalibration();

    /**
     * @brief 获取自动校准状态
     * 
     * @return true 正在校准中，false 校准已完成或未开始
     */
    bool isCalibrating() const;

    /**
     * @brief 获取自动校准进度
     * 
     * @return 当前采样进度（0-100%）
     */
    f32 getCalibrationProgress() const;

   private:
    /**
     * @brief 根据ADC分辨率将原始值转换为int16_t
     * 
     * 使用移位操作将不同分辨率的ADC原始值转换到int16_t正值范围(0到32767)
     * - 分辨率<=15位: 左移补齐到15位
     * - 分辨率>15位:  右移截取高15位
     */
    i16 _convertToInt16(u32 raw) const;

    /**
     * @brief 应用校准
     * 
     * @param value 待校准的值
     * @param channel 通道索引
     */
    i16 _applyCalibration(i16 value, u8 channel) const;

    /**
     * @brief 处理自动校准逻辑
     * 
     * @param currentTimeMs 当前时间（毫秒）
     * @param accumulator 各通道累加器（栈中分配）
     */
    void _processAutoCalibration(u32 currentTimeMs);

   private:
    Config     _config;                    ///< ADC配置
    u32        _maxAdcValue;               ///< ADC最大值
    i16        _values[CHANNELS];          ///< 各通道校准后的值
    const u16* _buffer;                    ///< 外部缓冲区指针
    i16        _channelOffsets[CHANNELS];  ///< 各通道独立的校准偏移

    // 自动校准相关
    bool                  _isCalibrating;     ///< 是否正在校准
    u16                   _calibSampleCount;  ///< 当前采样计数
    u32                   _lastSampleTime;    ///< 上次采样时间（毫秒）
    AutoCalibrationConfig _autoCalConfig;
    u32                   _calibAccumulator[CHANNELS];  ///< 校准累加器
};

// ============================================================================
// AnalogSource 模板实现
// ============================================================================

template <u8 CHANNELS>
AnalogSource<CHANNELS>::AnalogSource(const Config& config, const u16 buffer[CHANNELS])
    : _config(config),
      _buffer(buffer),
      _isCalibrating(false),
      _calibSampleCount(0),
      _lastSampleTime(0) {
    _maxAdcValue = (1U << _config.adcResolution) - 1;

    // 初始化各通道偏移为0
    for (u8 ch = 0; ch < CHANNELS; ch++) {
        _channelOffsets[ch] = 0;
    }

    reset();
}

template <u8 CHANNELS>
void AnalogSource<CHANNELS>::update() {
    if (_isCalibrating) {
        _processAutoCalibration(System::getTickMs());
    }

    // 从外部缓冲区读取原始值并转换
    if (_buffer != nullptr) {
        for (u8 ch = 0; ch < CHANNELS; ch++) {
            i16 converted = _convertToInt16(_buffer[ch]);
            _values[ch]   = _applyCalibration(converted, ch);
        }
    }
}

template <u8 CHANNELS>
void AnalogSource<CHANNELS>::reset() {
    for (u8 ch = 0; ch < CHANNELS; ch++) {
        _values[ch] = 0;
    }
    _isCalibrating    = false;
    _calibSampleCount = 0;
    _lastSampleTime   = 0;
}

template <u8 CHANNELS>
i16 AnalogSource<CHANNELS>::getValue(u8 channel) const {
    if (channel >= CHANNELS) {
        return 0;  // 返回无效值
    }
    return _values[channel];
}

template <u8 CHANNELS>
i16* AnalogSource<CHANNELS>::getValues() const {
    return const_cast<i16*>(_values);
}

template <u8 CHANNELS>
void AnalogSource<CHANNELS>::setCalibration(u8 channel, i16 offset) {
    _channelOffsets[channel] = offset;
}

template <u8 CHANNELS>
i16 AnalogSource<CHANNELS>::getCalibration(u8 channel) const {
    return _channelOffsets[channel];
}

template <u8 CHANNELS>
void AnalogSource<CHANNELS>::setBuffer(const u16 buffer[CHANNELS]) {
    _buffer = buffer;
}

template <u8 CHANNELS>
void AnalogSource<CHANNELS>::startAutoCalibration(const AutoCalibrationConfig& config) {
    _autoCalConfig    = config;
    _isCalibrating    = true;
    _calibSampleCount = 0;
    _lastSampleTime   = 0;
}

template <u8 CHANNELS>
void AnalogSource<CHANNELS>::stopAutoCalibration() {
    _isCalibrating    = false;
    _calibSampleCount = 0;
}

template <u8 CHANNELS>
bool AnalogSource<CHANNELS>::isCalibrating() const {
    return _isCalibrating;
}

template <u8 CHANNELS>
f32 AnalogSource<CHANNELS>::getCalibrationProgress() const {
    if (!_isCalibrating || _autoCalConfig.sampleCount == 0) {
        return 0.0f;
    }
    return (f32)_calibSampleCount / (f32)_autoCalConfig.sampleCount * 100.0f;
}

template <u8 CHANNELS>
i16 AnalogSource<CHANNELS>::_convertToInt16(u32 raw) const {
    // 将原始ADC值转换为int16_t范围 (0 到 32767)
    // 根据ADC分辨率进行移位操作:
    // - 8位ADC:  左移7位 (15-8=7)   0-255    -> 0-32640
    // - 10位ADC: 左移5位 (15-10=5)  0-1023   -> 0-32736
    // - 12位ADC: 左移3位 (15-12=3)  0-4095   -> 0-32760
    // - 16位ADC: 右移1位 (16-15=1)  0-65535  -> 0-32767

    if (_config.adcResolution <= 15) {
        // 左移: 目标位数更多
        u8 leftShift = 15 - _config.adcResolution;
        return static_cast<i16>(raw << leftShift);
    } else {
        // 右移: 目标位数更少
        u8 rightShift = _config.adcResolution - 15;
        return static_cast<i16>(raw >> rightShift);
    }
}

template <u8 CHANNELS>
i16 AnalogSource<CHANNELS>::_applyCalibration(i16 value, u8 channel) const {
    if (channel >= CHANNELS) return value;

    // 应用全局偏移和通道独立偏移: value = value + globalOffset + channelOffset
    i32 result = static_cast<i32>(value) + static_cast<i32>(_channelOffsets[channel]);

    // 限制到int16_t范围
    if (result > 32767) result = 32767;
    if (result < -32768) result = -32768;

    return static_cast<i16>(result);
}

template <u8 CHANNELS>
void AnalogSource<CHANNELS>::_processAutoCalibration(u32 currentTimeMs) {
    if (!_isCalibrating || _buffer == nullptr) {
        return;
    }

    // 检查是否到了采样时间
    if (currentTimeMs - _lastSampleTime >= _autoCalConfig.sampleIntervalMs) {
        _lastSampleTime = currentTimeMs;

        // 累加当前所有通道的原始值
        for (u8 ch = 0; ch < CHANNELS; ch++) {
            _calibAccumulator[ch] += _buffer[ch];
        }

        _calibSampleCount++;

        // 检查是否完成所有采样
        if (_calibSampleCount >= _autoCalConfig.sampleCount) {
            // 计算各通道的平均值作为偏移量
            for (u8 ch = 0; ch < CHANNELS; ch++) {
                u32 average         = _calibAccumulator[ch] / _autoCalConfig.sampleCount;
                // 将原始值平均转换为int16_t范围的偏移
                i16 avgConverted    = _convertToInt16(average);
                // 设置通道偏移量为负值，这样可以将当前值校准到0附近
                _channelOffsets[ch] = -avgConverted;
            }

            // 校准完成，重置累加器
            for (u8 ch = 0; ch < CHANNELS; ch++) {
                _calibAccumulator[ch] = 0;
            }

            _isCalibrating    = false;
            _calibSampleCount = 0;
        }
    }
}

}  // namespace wibot

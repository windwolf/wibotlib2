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

   public:
    /**
     * @brief 构造内存数据源
     * 
     * @param config ADC配置参数
     */
    explicit AnalogSource(const Config& config);

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
     * @brief 获取外部缓冲区
     * 
     * @return u16* 外部缓冲区指针
     */
    u16* getBuffer();

    /**
     * @brief 设置通道偏移量
     * 
     * @param channel 通道索引
     * @param offset 偏移量
     */
    void setOffset(u8 channel, i16 offset);

    /**
     * @brief 获取通道偏移量
     * 
     * @param channel 通道索引
     * @return i16 偏移量
     */
    i16 getOffset(u8 channel) const;

    /**
     * @brief 设置所有通道的偏移量
     * 
     * @param offsets 偏移量数组（长度必须为 CHANNELS）
     */
    void setOffsets(const i16 offsets[CHANNELS]);

    /**
     * @brief 获取所有通道的偏移量
     * 
     * @return const i16* 偏移量数组指针
     */
    const i16* getOffsets() const;

   private:
    /**
     * @brief 根据ADC分辨率将原始值转换为int16_t
     * 
     * 使用移位操作将不同分辨率的ADC原始值转换到int16_t正值范围(0到32767)
     * - 分辨率<=15位: 左移补齐到15位
     * - 分辨率>15位:  右移截取高15位
     */
    i16 _convertToInt16(u32 raw) const;

   private:
    Config _config;       ///< ADC配置
    u32    _maxAdcValue;  ///< ADC最大值

    u16 _buffer[CHANNELS];   ///< 原始值缓冲区
    i16 _values[CHANNELS];   ///< 各通道校准后的值
    i16 _offsets[CHANNELS];  ///< 各通道偏移量
};

// ============================================================================
// AnalogSource 模板实现
// ============================================================================

template <u8 CHANNELS>
AnalogSource<CHANNELS>::AnalogSource(const Config& config) : _config(config) {
    _maxAdcValue = (1U << _config.adcResolution) - 1;

    // 初始化偏移量为0
    for (u8 ch = 0; ch < CHANNELS; ch++) {
        _offsets[ch] = 0;
    }

    reset();
}

template <u8 CHANNELS>
void AnalogSource<CHANNELS>::update() {
    // 从外部缓冲区读取原始值并转换，然后应用偏移量
    for (u8 ch = 0; ch < CHANNELS; ch++) {
        i16 converted = _convertToInt16(_buffer[ch]);

        // 应用偏移校准: calibratedValue = rawValue + offset
        i32 result = static_cast<i32>(converted) + static_cast<i32>(_offsets[ch]);

        // 限制到 int16_t 范围
        if (result > 32767) result = 32767;
        if (result < -32768) result = -32768;

        _values[ch] = static_cast<i16>(result);
    }
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
void AnalogSource<CHANNELS>::reset() {
    for (u8 ch = 0; ch < CHANNELS; ch++) {
        _values[ch] = 0;
    }
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
u16* AnalogSource<CHANNELS>::getBuffer() {
    return _buffer;
}

template <u8 CHANNELS>
void AnalogSource<CHANNELS>::setOffset(u8 channel, i16 offset) {
    if (channel < CHANNELS) {
        _offsets[channel] = offset;
    }
}

template <u8 CHANNELS>
i16 AnalogSource<CHANNELS>::getOffset(u8 channel) const {
    if (channel >= CHANNELS) {
        return 0;
    }
    return _offsets[channel];
}

template <u8 CHANNELS>
void AnalogSource<CHANNELS>::setOffsets(const i16 offsets[CHANNELS]) {
    for (u8 ch = 0; ch < CHANNELS; ch++) {
        _offsets[ch] = offsets[ch];
    }
}

template <u8 CHANNELS>
const i16* AnalogSource<CHANNELS>::getOffsets() const {
    return _offsets;
}

}  // namespace wibot

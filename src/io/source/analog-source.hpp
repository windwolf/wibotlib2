#pragma once

#include "model.hpp"
#include "system.hpp"
#include <cstdint>

namespace wibot {

template <u8 CHANNELS>
class AnalogSource : public MultiChannelPipeline<i16, CHANNELS> {
   public:
    struct Storage {
        u16 rawValue[CHANNELS]{};  ///< 原始值缓冲区, 由外部(如DMA)填充
        i16 values[CHANNELS]{};    ///< 校准后的值
        i16 offsets[CHANNELS]{};   ///< 偏移量
    };

    struct Config {
        u8 resolution;  ///< ADC分辨率位数: 8=8位(0-255), 12=12位(0-4095), 16=16位(0-65535)
    };

   public:
    /**
     * @brief 构造多通道模拟输入源
     * 
     * @param config ADC配置参数
     */
    AnalogSource(const Config& config, Storage& storage) : _config(config), _storage(storage) {
        reset();
    }

    /**
     * @brief 更新所有通道数据
     */
    void update() override {
        // 从外部缓冲区读取原始值并转换，然后应用偏移量
        for (u8 ch = 0; ch < CHANNELS; ch++) {
            i16 converted = _convertToInt16(_storage.rawValue[ch]);

            // 应用偏移校准: calibratedValue = rawValue + offset
            i32 result = static_cast<i32>(converted) + static_cast<i32>(_storage.offsets[ch]);

            // 限制到 int16_t 范围
            if (result > 32767) result = 32767;
            if (result < -32768) result = -32768;

            _storage.values[ch] = static_cast<i16>(result);
        }
    }

    /**
     * @brief 获取指定通道的值
     * 
     * @param channel 通道索引
     * @return i16 通道值
     */
    i16 getValue(u8 channel) const override {
        if (channel >= CHANNELS) {
            return 0;  // 返回无效值
        }
        return _storage.values[channel];
    }

    /**
     * @brief 重置所有通道状态
     */
    void reset() override {
        for (u8 ch = 0; ch < CHANNELS; ch++) {
            _storage.values[ch]  = 0;
            _storage.offsets[ch] = 0;
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
     * @brief 获取外部缓冲区
     * 
     * @return u16* 外部缓冲区指针
     */
    u16* getBuffer() {
        return _storage.rawValue;
    }

    /**
     * @brief 设置通道偏移量
     * 
     * @param channel 通道索引
     * @param offset 偏移量
     */
    void setOffset(u8 channel, i16 offset) {
        if (channel < CHANNELS) {
            _storage.offsets[channel] = offset;
        }
    }

    /**
     * @brief 获取通道偏移量
     * 
     * @param channel 通道索引
     * @return i16 偏移量
     */
    i16 getOffset(u8 channel) const {
        if (channel >= CHANNELS) {
            return 0;
        }
        return _storage.offsets[channel];
    }

    /**
     * @brief 设置所有通道的偏移量
     * 
     * @param offsets 偏移量数组（长度必须为 CHANNELS）
     */
    void setOffsets(const i16 offsets[CHANNELS]) {
        for (u8 ch = 0; ch < CHANNELS; ch++) {
            _storage.offsets[ch] = offsets[ch];
        }
    }

    /**
     * @brief 获取所有通道的偏移量
     * 
     * @return const i16* 偏移量数组指针
     */
    const i16* getOffsets() const {
        return _storage.offsets;
    }

   private:
    /**
     * @brief 根据ADC分辨率将原始值转换为int16_t
     * 
     * 使用移位操作将不同分辨率的ADC原始值转换到int16_t正值范围(0到32767)
     * - 分辨率<=15位: 左移补齐到15位
     * - 分辨率>15位:  右移截取高15位
     */
    i16 _convertToInt16(u32 raw) const {
        // 将原始ADC值转换为int16_t范围 (0 到 32767)
        // 根据ADC分辨率进行移位操作:
        // - 8位ADC:  左移7位 (15-8=7)   0-255    -> 0-32640
        // - 10位ADC: 左移5位 (15-10=5)  0-1023   -> 0-32736
        // - 12位ADC: 左移3位 (15-12=3)  0-4095   -> 0-32760
        // - 16位ADC: 右移1位 (16-15=1)  0-65535  -> 0-32767

        if (_config.resolution <= 15) {
            // 左移: 目标位数更多
            u8 leftShift = 15 - _config.resolution;
            return static_cast<i16>(raw << leftShift);
        } else {
            // 右移: 目标位数更少
            u8 rightShift = _config.resolution - 15;
            return static_cast<i16>(raw >> rightShift);
        }
    }

   private:
    Config   _config;   ///< ADC配置
    Storage& _storage;  ///< 外部存储
};

}  // namespace wibot

#pragma once

#include "model.hpp"
#include <cstdint>

namespace wibot {

/**
 * @brief 常量值数据源
 * 
 * 用于生成可外部设置的常量值，支持多通道输出
 * 
 * @tparam T 数据类型
 * @tparam CHANNELS 通道数量
 */
template <typename T, u8 CHANNELS>
class ConstantSource : public SyncPipeline<T> {
   public:
    /**
     * @brief 构造常量数据源
     * 
     * @param defaultValue 默认常量值（所有通道初始值）
     */
    explicit ConstantSource(T defaultValue = T{}) {
        // 初始化所有通道为默认值
        for (u8 ch = 0; ch < CHANNELS; ch++) {
            _values[ch] = defaultValue;
        }
    }

    // Pipeline接口实现
    void update() override {
        // 常量源不需要更新操作，值保持不变
        // 这个方法为了实现接口而保留，实际不执行任何操作
    }
    T getValue(u8 channel) const override {
        if (channel >= CHANNELS) {
            return T{};  // 返回默认构造的值
        }
        return _values[channel];
    }
    T* getValues() const override {
        return const_cast<T*>(_values);
    }
    void reset() override {
        // 重置所有通道为默认构造的值
        for (u8 ch = 0; ch < CHANNELS; ch++) {
            _values[ch] = T{};
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
     * @brief 设置指定通道的常量值
     * 
     * @param channel 通道索引 (0 到 CHANNELS-1)
     * @param value 要设置的常量值
     */
    void setValue(u8 channel, T value) {
        if (channel < CHANNELS) {
            _values[channel] = value;
        }
    }

    /**
     * @brief 设置所有通道的常量值
     * 
     * @param value 要设置的常量值
     */
    void setAllValues(T value) {
        for (u8 ch = 0; ch < CHANNELS; ch++) {
            _values[ch] = value;
        }
    }

    /**
     * @brief 设置多个通道的常量值
     * 
     * @param values 包含各通道常量值的数组，长度必须为 CHANNELS
     */
    void setValues(const T values[CHANNELS]) {
        for (u8 ch = 0; ch < CHANNELS; ch++) {
            _values[ch] = values[ch];
        }
    }

    /**
     * @brief 获取指定通道的当前常量值
     * 
     * @param channel 通道索引 (0 到 CHANNELS-1)
     * @return T 当前设置的常量值，如果通道索引无效则返回默认构造的值
     */
    T getConstantValue(u8 channel) const {
        if (channel >= CHANNELS) {
            return T{};  // 返回默认构造的值
        }
        return _values[channel];
    }

   private:
    T _values[CHANNELS];  ///< 各通道的常量值
};

}  // namespace wibot
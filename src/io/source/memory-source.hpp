#pragma once

#include "../model.hpp"
#include <cstdint>

namespace wibot {

/**
 * @brief 多通道内存数据源
 * 
 * 为多通道硬件提供内存存储的数据源。
 * 每个通道独立维护数据值，可用于缓存或暂存多通道数据。
 * 
 * @tparam T 数据类型
 * @tparam CHANNELS 通道数量，编译时确定
 */
template <typename T, u8 CHANNELS>
class MemorySource : public MultiChannelPipeline<T, CHANNELS> {
   public:
    struct Storage {
        T values[CHANNELS]{};
    };

    /**
     * @brief 构造多通道内存数据源
     * 
     * @param storage 存储器引用
     * @param defaultValue 所有通道的默认值
     */
    MemorySource(Storage& storage, T defaultValue = T{})
        : _storage(storage), _defaultValue(defaultValue) {
        reset();
    }

    void update() override {
        // 多通道内存源不需要更新操作，值保持不变
    }

    T getValue(u8 channel) const override {
        if (channel < CHANNELS) {
            return _storage.values[channel];
        }
        return T{};
    }

    void reset() override {
        // 重置所有通道为默认值
        for (u8 i = 0; i < CHANNELS; ++i) {
            _storage.values[i] = _defaultValue;
        }
    }

   private:
    Storage& _storage;       ///< 多通道数据存储
    T        _defaultValue;  ///< 默认值
};

}  // namespace wibot

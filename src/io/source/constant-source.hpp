#pragma once

#include "model.hpp"
#include <cstdint>

namespace wibot {

/**
 * @brief 常量值数据源
 * 
 * 用于生成可外部设置的常量值。
 * 
 * @tparam T 数据类型
 */
template <typename T>
class ConstantSource : public SyncPipeline<T> {
   public:
    /**
     * @brief 构造常量数据源
     * 
     * @param defaultValue 默认常量值
     */
    explicit ConstantSource(T defaultValue = T{}) : _value(defaultValue) {
    }

    void update() override {
        // 常量源不需要更新操作，值保持不变
    }

    T getValue() const override {
        return _value;
    }

    void reset() override {
        // 重置为默认构造的值
        _value = T{};
    }

   public:
    /**
     * @brief 设置常量值
     * 
     * @param value 要设置的常量值
     */
    void setValue(T value) {
        _value = value;
    }

   private:
    T _value;  ///< 常量值
};

}  // namespace wibot
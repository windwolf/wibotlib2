#pragma once

#include "../pipeline.hpp"

namespace wibot {

/**
 * @brief 常量源 INode 实现
 * 
 * 模板化支持任意类型
 */
template <typename T>
class ConstantSourceNode : public INode {
   public:
    struct Outputs {
        Out<T> x;
    } outputs;

    /**
     * @brief 构造函数
     * @param defaultValue 默认值
     */
    explicit ConstantSourceNode(T defaultValue = T{}) : _value(defaultValue) {
    }

    bool ready() override {
        return true;
    }

    void process() override {
        if (outputs.x.bound()) {
            outputs.x.ref() = _value;
        }
    }

    void reset() override {
        _value = T{};
    }

    /**
     * @brief 设置常量值
     */
    void setValue(T value) {
        _value = value;
    }

   private:
    T _value;
};

}  // namespace wibot

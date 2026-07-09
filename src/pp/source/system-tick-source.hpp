#pragma once

#include "../pipeline.hpp"
#include "hal/system.hpp"

namespace wibot {

/**
 * @brief 系统时间源节点
 * 
 * 输出当前系统时间（毫秒）
 */
class SystemTickSourceNode : public INode {
   public:
    struct Outputs {
        Out<u32> tick;  // 系统时间戳（毫秒）
    } outputs;

    /**
     * @brief 构造函数
     */
    SystemTickSourceNode() = default;

    bool ready() override {
        return true;
    }

    void process() override {
        if (outputs.tick.bound()) {
            outputs.tick.ref() = System::getTickMs();
        }
    }

    void reset() override {
        // 无需重置，tick 由系统维护
    }
};

}  // namespace wibot

#pragma once

#include "base/type.hpp"

namespace wibot {

/**
 * @brief 编码器位置跟踪器
 * 
 * 根据编码器原始值计算累积位置，处理折叠（wrap）问题
 */
class PositionTracker {
   public:
    struct Config {
        // 编码器物理参数
        u32 resolution;  // 编码器分辨率 (ticks/rev)

        // 输入数据折叠特性
        u32 inputWrapRange;  // 输入折叠范围；为0表示无需折叠
    };

   public:
    /**
     * @brief 更新位置跟踪
     * 
     * @param value 当前编码器读数 (ticks)
     */
    void update(u32 value);

    /**
     * @brief 重置位置跟踪器
     * 
     * @param value 当前编码器读数 (ticks)
     * @param position 初始位置 (ticks)
     */
    void reset(u32 value, i32 position = 0);

    /**
     * @brief 获取当前位置 (ticks)
     */
    i32 getPosition() const {
        return _position;
    }

    /**
     * @brief 获取换算后的角度值 (弧度)
     */
    f32 getAngular() const;

    /**
     * @brief 获取最近一次位移 (ticks)
     */
    i32 getLastDisplacement() const {
        return _lastDisplacement;
    }

    explicit PositionTracker(Config& config) : _config(config) {
    }

   private:
    Config& _config;

    u32 _lastValue        = 0;  // 上一次的编码器读数
    i32 _position         = 0;  // 当前位置 (ticks)
    i32 _lastDisplacement = 0;  // 最近一次位移 (ticks)

   private:
    /**
     * @brief 处理位移，考虑环绕
     */
    i32 _calculateDisplacement(u32 currentValue);
};

}  // namespace wibot

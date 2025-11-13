#pragma once

#include "model.hpp"
#include "type.hpp"
#include <cmath>

namespace wibot {

/**
 * @brief 斜坡轨迹生成器配置参数
 */
struct SlopeTrajectoryConfig {
    f32 slopeRate;   ///< 斜坡速率 (单位/秒)
    f32 sampleTime;  ///< 采样时间 (秒)
};

/**
 * @brief 斜坡轨迹生成器管道
 * 
 * 实现SyncPipeline接口的斜坡轨迹生成器，从上游管道获取设定值，
 * 按照指定的斜坡速率从当前值逐渐变化到设定值。
 * 
 * 特性：
 * - 支持多通道并行处理
 * - 可配置斜坡速率
 * - 当设定值改变时，自动从当前输出值开始斜坡变化
 * - 支持正向和反向斜坡
 * 
 * @tparam CHANNELS 通道数量，必须大于0
 */
template <u8 CHANNELS = 1>
class SlopeTrajectory : public SyncPipeline<f32, f32*> {
   public:
    /**
     * @brief 构造函数
     * @param upstream 上游管道，提供设定值
     */
    explicit SlopeTrajectory(SyncPipeline<f32, f32*>* upstream) : _upstream(upstream), _config() {
        // 初始化所有通道的状态
        for (u8 i = 0; i < CHANNELS; ++i) {
            _outputs[i]   = 0.0f;
            _setPoints[i] = 0.0f;
        }
    };

    /**
     * @brief 更新管道状态
     * 
     * 从上游管道获取新的设定值，并根据斜坡速率更新输出值。
     * 如果设定值发生变化，将从当前输出值开始斜坡变化到新设定值。
     */
    void update() override {
        if (_upstream == nullptr) {
            // 没有上游管道，保持当前输出值不变
            return;
        }

        // 更新上游管道
        _upstream->update();

        // 处理所有通道
        for (u8 channel = 0; channel < CHANNELS; ++channel) {
            // 获取该通道的设定值
            f32 setPoint = _upstream->getValue(channel);
            updateChannel(channel, setPoint);
        }
    };

    /**
     * @brief 获取指定通道的输出值
     * @param channel 通道索引 (0 到 CHANNELS-1)
     * @return 当前输出值
     */
    f32 getValue(u8 channel) const override {
        if (channel >= CHANNELS) {
            return 0.0f;  // 超出范围返回0
        }
        return _outputs[channel];
    };

    /**
     * @brief 获取所有通道的输出值数组
     * @return 输出值数组指针
     */
    f32* getValues() const override {
        return const_cast<f32*>(_outputs);
    };

    /**
     * @brief 重置管道状态
     * 
     * 将所有通道的输出值和设定值重置为0。
     */
    void reset() override {
        for (u8 i = 0; i < CHANNELS; ++i) {
            _outputs[i]   = 0.0f;
            _setPoints[i] = 0.0f;
        }
    };

    /**
     * @brief 设置配置参数
     * @param config 配置参数
     */
    void setConfig(const SlopeTrajectoryConfig& config) {
        _config = config;

        // 确保参数合理性
        if (_config.slopeRate < 0) {
            _config.slopeRate = 0.1f;  // 设置最小斜坡速率
        }

        if (_config.sampleTime <= 0) {
            _config.sampleTime = 0.01f;  // 设置默认采样时间
        }
    };

    /**
     * @brief 获取当前配置参数
     * @return 当前配置参数
     */
    const SlopeTrajectoryConfig& getConfig() const {
        return _config;
    };

    /**
     * @brief 设置上游管道
     * @param upstream 上游管道指针
     */
    void setUpstream(SyncPipeline<f32, f32*>* upstream) {
        _upstream = upstream;
    };

    /**
     * @brief 设置指定通道的初始输出值
     * @param channel 通道索引
     * @param value 初始值
     */
    void setInitialValue(u8 channel, f32 value) {
        if (channel < CHANNELS) {
            _outputs[channel]   = clampValue(value);
            _setPoints[channel] = _outputs[channel];
        }
    };

    /**
     * @brief 设置所有通道的初始输出值
     * @param values 初始值数组，长度必须为CHANNELS
     */
    void setInitialValues(const f32* values) {
        if (values == nullptr) {
            return;
        }

        for (u8 i = 0; i < CHANNELS; ++i) {
            setInitialValue(i, values[i]);
        }
    };

    /**
     * @brief 检查指定通道是否已到达设定值
     * @param channel 通道索引
     * @param tolerance 容差值，默认为1e-6
     * @return true 如果已到达设定值
     */
    bool isReached(u8 channel, f32 tolerance = 1e-6f) const {
        if (channel >= CHANNELS) {
            return false;
        }

        return std::abs(_outputs[channel] - _setPoints[channel]) <= tolerance;
    };

    /**
     * @brief 检查所有通道是否都已到达设定值
     * @param tolerance 容差值，默认为1e-6
     * @return true 如果所有通道都已到达设定值
     */
    bool allReached(f32 tolerance = 1e-6f) const {
        for (u8 i = 0; i < CHANNELS; ++i) {
            if (!isReached(i, tolerance)) {
                return false;
            }
        }
        return true;
    };

   private:
    SyncPipeline<f32, f32*>* _upstream;  ///< 上游管道指针
    SlopeTrajectoryConfig    _config;    ///< 配置参数

    f32 _outputs[CHANNELS];    ///< 当前输出值数组
    f32 _setPoints[CHANNELS];  ///< 当前设定值数组

    /**
     * @brief 更新单个通道的输出值
     * @param channel 通道索引
     * @param setPoint 设定值
     */
    void updateChannel(u8 channel, f32 setPoint) {
        if (channel >= CHANNELS) {
            return;  // 超出范围，忽略
        }

        // 更新设定值
        _setPoints[channel] = setPoint;

        // 计算当前输出值与设定值的差值
        f32 error = _setPoints[channel] - _outputs[channel];

        // 如果误差很小，直接设置为设定值
        if (std::abs(error) < 1e-6f) {
            _outputs[channel] = _setPoints[channel];
            return;
        }

        // 计算本次更新的最大变化量
        f32 maxChange = _config.slopeRate * _config.sampleTime;

        // 根据误差方向确定变化量
        f32 change;
        if (error > 0) {
            // 需要增加
            change = std::min(error, maxChange);
        } else {
            // 需要减少
            change = std::max(error, -maxChange);
        }

        // 更新输出值
        _outputs[channel] += change;

        // 限制输出值在合理范围内
        _outputs[channel] = clampValue(_outputs[channel]);
    };

    /**
     * @brief 限制数值在合理范围内
     * @param value 输入值
     * @return 限制后的值
     */
    f32 clampValue(f32 value) const {
        // 检查是否为有效数值
        if (std::isnan(value) || std::isinf(value)) {
            return 0.0f;
        }

        // 限制在合理范围内，防止数值溢出
        const f32 maxValue = 1e6f;
        const f32 minValue = -1e6f;

        return std::max(minValue, std::min(maxValue, value));
    };
};

}  // namespace wibot
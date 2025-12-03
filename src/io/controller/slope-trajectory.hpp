#pragma once

#include "model.hpp"
#include "type.hpp"
#include <cmath>
#include <type_traits>
#include <limits>

namespace wibot {

/**
 * @brief 斜坡轨迹生成器基础配置（非模板部分）
 */
struct SlopeTrajectoryBaseConfig {
    f32 slopeRate;   ///< 斜坡速率 (单位/秒)
    f32 sampleTime;  ///< 采样时间 (秒)

    SlopeTrajectoryBaseConfig() : slopeRate(1.0f), sampleTime(0.01f) {
    }
};

/**
 * @brief 斜坡轨迹生成器配置参数
 * 
 * @tparam T 数值类型
 */
template <typename T>
struct SlopeTrajectoryConfig : public SlopeTrajectoryBaseConfig {
    T minValue;  ///< 最小值限制
    T maxValue;  ///< 最大值限制

    SlopeTrajectoryConfig() : SlopeTrajectoryBaseConfig() {
        if constexpr (std::is_floating_point_v<T>) {
            minValue = -std::numeric_limits<T>::max();
            maxValue = std::numeric_limits<T>::max();
        } else {
            minValue = std::numeric_limits<T>::min();
            maxValue = std::numeric_limits<T>::max();
        }
    }
};

/**
 * @brief 斜坡轨迹生成器管道
 * 
 * 实现SyncPipeline接口的斜坡轨迹生成器，从上游管道获取设定值，
 * 按照指定的斜坡速率从当前值逐渐变化到设定值。
 * 
 * 特性：
 * - 支持多通道并行处理
 * - 支持多种数值类型：u8, u16, u32, i8, i16, i32, f32, f64
 * - 可配置斜坡速率和数值范围限制
 * - 当设定值改变时，自动从当前输出值开始斜坡变化
 * - 支持正向和反向斜坡
 * - 整数类型具有防溢出保护
 * 
 * @tparam T 数值类型，支持 u8, u16, u32, i8, i16, i32, f32, f64
 * @tparam CHANNELS 通道数量，必须大于0
 */
template <typename T, u8 CHANNELS = 1>
class SlopeTrajectory : public SyncPipeline<T, T*> {
    static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");
    static_assert(CHANNELS > 0, "CHANNELS must be greater than 0");

   public:
    /**
     * @brief 构造函数
     * @param upstream 上游管道，提供设定值
     */
    explicit SlopeTrajectory(SyncPipeline<T, T*>& upstream) : _upstream(upstream), _config() {
        // 初始化所有通道的状态
        for (u8 i = 0; i < CHANNELS; ++i) {
            _outputs[i]   = T{0};
            _setPoints[i] = T{0};
        }
    };

    /**
     * @brief 更新管道状态
     * 
     * 从上游管道获取新的设定值，并根据斜坡速率更新输出值。
     * 如果设定值发生变化，将从当前输出值开始斜坡变化到新设定值。
     */
    ALWAYS_INLINE void update() override {
        // 更新上游管道
        _upstream.update();

        // 处理所有通道
        for (u8 channel = 0; channel < CHANNELS; ++channel) {
            // 获取该通道的设定值
            T setPoint = _upstream.getValue(channel);
            updateChannel(channel, setPoint);
        }
    };

    /**
     * @brief 获取指定通道的输出值
     * @param channel 通道索引 (0 到 CHANNELS-1)
     * @return 当前输出值
     */
    ALWAYS_INLINE T getValue(u8 channel) const override {
        // 编译期常量检查以优化性能
        if constexpr (CHANNELS == 1) {
            return _outputs[0];  // 单通道优化
        } else {
            return (channel < CHANNELS) ? _outputs[channel] : T{0};
        }
    };

    /**
     * @brief 获取所有通道的输出值数组
     * @return 输出值数组指针
     */
    ALWAYS_INLINE T* getValues() const override {
        return const_cast<T*>(_outputs);
    };

    /**
     * @brief 重置管道状态
     * 
     * 将所有通道的输出值和设定值重置为0。
     */
    void reset() override {
        for (u8 i = 0; i < CHANNELS; ++i) {
            _outputs[i]   = T{0};
            _setPoints[i] = T{0};
        }
    };

    /**
     * @brief 设置配置参数
     * @param config 配置参数
     */
    void setConfig(const SlopeTrajectoryConfig<T>& config) {
        _config = config;

        // 确保参数合理性
        if (_config.slopeRate < 0) {
            _config.slopeRate = 0.1f;  // 设置最小斜坡速率
        }

        if (_config.sampleTime <= 0) {
            _config.sampleTime = 0.01f;  // 设置默认采样时间
        }

        // 确保范围合理性
        if (_config.minValue > _config.maxValue) {
            _config.minValue = std::numeric_limits<T>::min();
            _config.maxValue = std::numeric_limits<T>::max();
        }
    };

    /**
     * @brief 获取当前配置参数
     * @return 当前配置参数
     */
    const SlopeTrajectoryConfig<T>& getConfig() const {
        return _config;
    };

    /**
     * @brief 设置指定通道的初始输出值
     * @param channel 通道索引
     * @param value 初始值
     */
    void setInitialValue(u8 channel, T value) {
        if (channel < CHANNELS) {
            _outputs[channel]   = clampValue(value);
            _setPoints[channel] = _outputs[channel];
        }
    };

    /**
     * @brief 设置所有通道的初始输出值
     * @param values 初始值数组，长度必须为CHANNELS
     */
    void setInitialValues(const T* values) {
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
     * @param tolerance 容差值，对于整数类型默认为0，浮点类型默认为1e-6
     * @return true 如果已到达设定值
     */
    bool isReached(u8 channel, T tolerance = getDefaultTolerance()) const {
        if constexpr (CHANNELS == 1) {
            // 单通道时直接跳过边界检查
        } else if (channel >= CHANNELS) {
            return false;
        }

        if constexpr (std::is_integral_v<T>) {
            return _outputs[channel] == _setPoints[channel];
        } else {
            return std::abs(_outputs[channel] - _setPoints[channel]) <= tolerance;
        }
    };

    /**
     * @brief 检查所有通道是否都已到达设定值
     * @param tolerance 容差值，对于整数类型默认为0，浮点类型默认为1e-6
     * @return true 如果所有通道都已到达设定值
     */
    bool allReached(T tolerance = getDefaultTolerance()) const {
        for (u8 i = 0; i < CHANNELS; ++i) {
            if (!isReached(i, tolerance)) {
                return false;
            }
        }
        return true;
    };

   private:
    SyncPipeline<T, T*>&     _upstream;  ///< 上游管道引用
    SlopeTrajectoryConfig<T> _config;    ///< 配置参数

    T _outputs[CHANNELS];    ///< 当前输出值数组
    T _setPoints[CHANNELS];  ///< 当前设定值数组

    /**
     * @brief 更新单个通道的输出值
     * @param channel 通道索引
     * @param setPoint 设定值
     */
    void updateChannel(u8 channel, T setPoint) {
        if (channel >= CHANNELS) {
            return;  // 超出范围，忽略
        }

        // 更新设定值
        _setPoints[channel] = clampValue(setPoint);

        if constexpr (std::is_integral_v<T>) {
            // 整数类型处理
            updateChannelInteger(channel);
        } else {
            // 浮点类型处理
            updateChannelFloat(channel);
        }
    };

    /**
     * @brief 更新整数类型通道
     * @param channel 通道索引
     */
    void updateChannelInteger(u8 channel) {
        // 计算当前输出值与设定值的差值
        using SignedT = std::make_signed_t<T>;
        SignedT error =
            static_cast<SignedT>(_setPoints[channel]) - static_cast<SignedT>(_outputs[channel]);

        if (error == 0) {
            return;  // 已到达目标
        }

        // 计算本次更新的最大变化量（使用f32避免f64开销）
        f32 maxChangeFloat = _config.slopeRate * _config.sampleTime;

        // 确保至少变化1个单位
        SignedT maxChange = std::max(static_cast<SignedT>(1), static_cast<SignedT>(maxChangeFloat));

        // 根据误差方向确定变化量
        SignedT change;
        if (error > 0) {
            change = std::min(error, maxChange);
        } else {
            change = std::max(error, static_cast<SignedT>(-maxChange));
        }

        // 安全地更新输出值，防止溢出
        SignedT newValue = static_cast<SignedT>(_outputs[channel]) + change;
        if (newValue > static_cast<SignedT>(_config.maxValue)) {
            _outputs[channel] = _config.maxValue;
        } else if (newValue < static_cast<SignedT>(_config.minValue)) {
            _outputs[channel] = _config.minValue;
        } else {
            _outputs[channel] = static_cast<T>(newValue);
        }

        // 最终限制
        _outputs[channel] = clampValue(_outputs[channel]);
    };

    /**
     * @brief 更新浮点类型通道
     * @param channel 通道索引
     */
    void updateChannelFloat(u8 channel) {
        // 计算当前输出值与设定值的差值
        T error = _setPoints[channel] - _outputs[channel];

        // 如果误差很小，直接设置为设定值
        if (std::abs(error) < getDefaultTolerance()) {
            _outputs[channel] = _setPoints[channel];
            return;
        }

        // 计算本次更新的最大变化量
        T maxChange = static_cast<T>(_config.slopeRate * _config.sampleTime);

        // 根据误差方向确定变化量
        T change;
        if (error > 0) {
            change = std::min(error, maxChange);
        } else {
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
    ALWAYS_INLINE T clampValue(T value) const {
        if constexpr (std::is_floating_point_v<T>) {
            // 浮点类型：检查是否为有效数值
            if (std::isnan(value) || std::isinf(value)) {
                return T{0};
            }
        }

        // 限制在配置的范围内
        return std::max(_config.minValue, std::min(_config.maxValue, value));
    };

    /**
     * @brief 获取默认容差值
     * @return 对于整数类型返回0，浮点类型返回适当的小值
     */
    ALWAYS_INLINE static constexpr T getDefaultTolerance() {
        if constexpr (std::is_integral_v<T>) {
            return T{0};
        } else {
            return static_cast<T>(1e-6);
        }
    };
};

}  // namespace wibot
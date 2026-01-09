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
    f32  slopeRate;    ///< 斜坡速率 (单位/秒)
    f32  sampleTime;   ///< 采样时间 (秒)
    bool enableClamp;  ///< 是否启用最大最小值钳位
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
};

/**
 * @brief 斜坡轨迹生成器管道
 * 
 * 实现SyncPipeline接口的斜坡轨迹生成器，从上游管道获取设定值，
 * 按照指定的斜坡速率从当前值逐渐变化到设定值。
 * 
 * 特性：
 * - 支持多种数值类型：u8, u16, u32, i8, i16, i32, f32, f64
 * - 可配置斜坡速率和数值范围限制
 * - 当设定值改变时，自动从当前输出值开始斜坡变化
 * - 支持正向和反向斜坡
 */

/**
 * @brief 斜坡轨迹生成器管道
 * 
 * 将目标设定值通过可配置的斜率平滑过渡,避免突变。
 * 
 * @tparam T 数值类型，支持 u8, u16, u32, i8, i16, i32, f32, f64
 */
template <typename T>
class SlopeTrajectory : public SyncPipeline<T> {
    static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");

   public:
    struct Storage {
        T output{static_cast<T>(0)};
        T setPoint{static_cast<T>(0)};
    };

    /**
     * @brief 构造函数
     * @param upstream 上游管道，提供设定值
     * @param config 配置参数
     */
    explicit SlopeTrajectory(SyncPipeline<T>& upstream, SlopeTrajectoryConfig<T>& config,
                             Storage& storage)
        : _upstream(upstream), _config(config), _storage(storage) {
        _storage.output   = T{0};
        _storage.setPoint = T{0};
    };

    /**
     * @brief 更新管道状态
     * 
     * 从上游管道获取新的设定值，并根据斜坡速率更新输出值。
     * 如果设定值发生变化，将从当前输出值开始斜坡变化到新设定值。
     */
    void update() override {
        _upstream.update();
        T setPoint = _upstream.getValue();
        updateSlope(setPoint);
    };

    /**
     * @brief 获取输出值
     * @return 当前输出值
     */
    ALWAYS_INLINE T getValue() const override {
        return _storage.output;
    };

    /**
     * @brief 重置管道状态
     * 
     * 将输出值和设定值重置为0。
     */
    void reset() override {
        _storage.output   = T{0};
        _storage.setPoint = T{0};
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
     * @brief 设置初始输出值
     * @param value 初始值
     */
    void setInitialValue(T value) {
        _storage.output   = clampValue(value);
        _storage.setPoint = _storage.output;
    };

    /**
     * @brief 检查是否已到达设定值
     * @param tolerance 容差值（仅浮点类型有效）
     * @return true 如果已到达设定值
     */
    template <typename U = T>
    bool isReached(
        typename std::enable_if_t<std::is_floating_point_v<U>, f32> tolerance = 1e-6f) const {
        return std::abs(_storage.output - _storage.setPoint) <= tolerance;
    }

    template <typename U = T>
    bool isReached(typename std::enable_if_t<std::is_integral_v<U>, int> = 0) const {
        return _storage.output == _storage.setPoint;
    };

   private:
    SyncPipeline<T>&          _upstream;  ///< 上游管道引用
    SlopeTrajectoryConfig<T>& _config;    ///< 配置参数
    Storage&                  _storage;   ///< 外部存储

    /**
     * @brief 更新斜坡输出值
     * @param setPoint 设定值
     */
    void updateSlope(T setPoint) {
        _storage.setPoint = clampValue(setPoint);

        if constexpr (std::is_integral_v<T>) {
            updateSlopeInteger();
        } else {
            updateSlopeFloat();
        }
    };

    /**
     * @brief 更新浮点类型
     */
    void updateSlopeFloat() {
        T error = _storage.setPoint - _storage.output;

        if (std::abs(error) < static_cast<T>(1e-9)) {
            _storage.output = _storage.setPoint;
            return;
        }

        T maxChange = static_cast<T>(_config.slopeRate * _config.sampleTime);

        T change;
        if (error > 0) {
            change = std::min(error, maxChange);
        } else {
            change = std::max(error, -maxChange);
        }

        _storage.output += change;
    };

    /**
     * @brief 更新整数类型
     */
    void updateSlopeInteger() {
        using SignedT = std::make_signed_t<T>;
        SignedT error =
            static_cast<SignedT>(_storage.setPoint) - static_cast<SignedT>(_storage.output);

        if (error == 0) {
            return;
        }

        f32     maxChangeFloat = _config.slopeRate * _config.sampleTime;
        SignedT maxChange = std::max(static_cast<SignedT>(1), static_cast<SignedT>(maxChangeFloat));

        SignedT change;
        if (error > 0) {
            change = std::min(error, maxChange);
        } else {
            change = std::max(error, static_cast<SignedT>(-maxChange));
        }

        SignedT newValue = static_cast<SignedT>(_storage.output) + change;
        if (newValue > static_cast<SignedT>(_config.maxValue)) {
            _storage.output = _config.maxValue;
        } else if (newValue < static_cast<SignedT>(_config.minValue)) {
            _storage.output = _config.minValue;
        } else {
            _storage.output = static_cast<T>(newValue);
        }

        _storage.output = clampValue(_storage.output);
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

        // 如果启用钳位，限制在配置的范围内
        if (_config.enableClamp) {
            return std::max(_config.minValue, std::min(_config.maxValue, value));
        }
        return value;
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
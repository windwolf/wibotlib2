#pragma once

#include "type.hpp"
#include <algorithm>
#include <cmath>
#include <type_traits>
#include <limits>
#include <concepts>

namespace wibot {

template <typename T>
    requires SupportArithmetic<T>
class SlopeTrajectory {
   public:
    struct Config {
        f32  slopeRate{0.0f};              // 斜坡速率 (单位/秒)
        bool enableClamp{false};           // 是否启用最大最小值钳位
        T    minValue{static_cast<T>(0)};  // 最小值限制
        T    maxValue{static_cast<T>(0)};  // 最大值限制
    };
    struct State {
        T output{static_cast<T>(0)};
        T setPoint{static_cast<T>(0)};
    };

   public:
    explicit SlopeTrajectory(Config& config) : _config(config), _state() {
    }

    void reset() {
        _state.output   = static_cast<T>(0);
        _state.setPoint = static_cast<T>(0);
    }

    void setInitialValue(T value) {
        _state.output   = clampValue(value);
        _state.setPoint = _state.output;
    }

    T update(T setPoint, f32 samplePeriod)
        requires SupportFloat<T>
    {
        _state.setPoint = clampValue(setPoint);
        T error         = _state.setPoint - _state.output;
        if (std::abs(error) < static_cast<T>(1e-9)) {
            _state.output = _state.setPoint;
            return _state.output;
        }

        if (samplePeriod <= 0.0f) {
            return _state.output;  // 采样周期无效，不更新
        }

        T maxChange = static_cast<T>(_config.slopeRate * samplePeriod);
        T change    = (error > 0) ? std::min(error, maxChange) : std::max(error, -maxChange);
        _state.output += change;
        return _state.output;
    }

    T update(T setPoint, f32 samplePeriod)
        requires SupportInt<T> or SupportUint<T>
    {
        _state.setPoint = clampValue(setPoint);

        if (samplePeriod <= 0.0f) {
            return _state.output;  // 采样周期无效，不更新
        }

        // 计算最大变化量
        T maxChange = static_cast<T>(_config.slopeRate * samplePeriod);
        if (maxChange == 0) {
            maxChange = 1;  // 最小步进
        }

        // 使用比较避免无符号数下溢
        if (_state.setPoint > _state.output) {
            T error = _state.setPoint - _state.output;
            _state.output += std::min(error, maxChange);
        } else if (_state.setPoint < _state.output) {
            T error = _state.output - _state.setPoint;
            _state.output -= std::min(error, maxChange);
        }
        // 否则相等，无需改变

        return _state.output;
    }

    bool isReached(f32 tolerance = 1e-6f) const
        requires std::is_floating_point_v<T>
    {
        return std::abs(_state.output - _state.setPoint) <= tolerance;
    }

    bool isReached() const
        requires std::is_integral_v<T>
    {
        return _state.output == _state.setPoint;
    }

   private:
    T clampValue(T value) const {
        if (!_config.enableClamp) {
            return value;
        }
        if (_config.minValue > _config.maxValue) {
            return value;
        }
        if constexpr (std::is_floating_point_v<T>) {
            if (std::isnan(value) || std::isinf(value)) {
                return static_cast<T>(0);
            }
        }
        if (value < _config.minValue) return _config.minValue;
        if (value > _config.maxValue) return _config.maxValue;
        return value;
    }

   private:
    Config& _config{};
    State   _state;
};

template <typename T>
    requires SupportInt<T> or SupportUint<T>
class SlopeTrajectoryFast {
   public:
    struct Config {
        u32  slopeRate;                    // 斜坡速率 (单位/秒)
        bool enableClamp{false};           // 是否启用最大最小值钳位
        T    minValue{static_cast<T>(0)};  // 最小值限制
        T    maxValue{static_cast<T>(0)};  // 最大值限制
    };
    struct State {
        T output{static_cast<T>(0)};
        T setPoint{static_cast<T>(0)};
    };

   public:
    explicit SlopeTrajectoryFast(Config& config) : _config(config), _state() {
    }

    void reset() {
        _state.output   = static_cast<T>(0);
        _state.setPoint = static_cast<T>(0);
    }

    void setInitialValue(T value) {
        _state.output   = clampValue(value);
        _state.setPoint = _state.output;
    }

    T update(T setPoint, u32 samplePeriodMs) {
        _state.setPoint = clampValue(setPoint);

        // 计算最大变化量
        T maxChange = static_cast<T>(_config.slopeRate * samplePeriodMs / 1000);
        if (maxChange == 0) {
            maxChange = 1;  // 最小步进
        }

        // 使用比较避免无符号数下溢
        if (_state.setPoint > _state.output) {
            T error = _state.setPoint - _state.output;
            _state.output += std::min(error, maxChange);
        } else if (_state.setPoint < _state.output) {
            T error = _state.output - _state.setPoint;
            _state.output -= std::min(error, maxChange);
        }
        // 否则相等，无需改变

        return _state.output;
    }

    bool isReached() const
        requires std::is_integral_v<T>
    {
        return _state.output == _state.setPoint;
    }

   private:
    T clampValue(T value) const {
        if (!_config.enableClamp) {
            return value;
        }
        if (_config.minValue > _config.maxValue) {
            return value;
        }
        if (value < _config.minValue) return _config.minValue;
        if (value > _config.maxValue) return _config.maxValue;
        return value;
    }

   private:
    Config& _config{};
    State   _state;
};

}  // namespace wibot

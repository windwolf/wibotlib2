#pragma once

//
// Created by zhouj on 2022/12/10.
//

#include <cmath>

#include "value-mapper.hpp"
namespace wibot {

#define PIECEWISE_LINEAR_VALUE_MAPPER_ENABLE_K 0

template <size_t N>
struct PiecewiseLinearValueMapperConfig {
    u32 zeroOffset;
    u32 inWrap;
    f32 outWrap;
    u32 inPoints[N];
    f32 outPoints[N];
#if PIECEWISE_LINEAR_VALUE_MAPPER_ENABLE_K
    // f32 out_k[N];
#endif
};

/**
 * @brief Piecewise linear value mapper
 * Map raw value to zero to _wrap value, with zero offset.
 * @tparam N
 */
template <size_t N>
class PiecewiseLinearValueMapper : public ValueMapper {
   public:
    /**
     *	@brief	开始校准.
     *	@note Make sure the zero offset is correct before calibration.
     */
    void beginCalirate();
    void calibrate(u8 step, u32 in_value, f32 out_value);
    void endCalibrate();
    void setZeroOffset(u32 zero_offset);
    f32  getValue(u32 in_value) override;

   public:
    PiecewiseLinearValueMapperConfig<N> config;

   private:
    f32 _lastValue;

    u8 _calCount[N];

   private:
    u32 _wrap(i32 value);
};
template <size_t N>
void PiecewiseLinearValueMapper<N>::endCalibrate() {
    for (u32 i = 0; i < N - 1; ++i) {
        this->config.inPoints[i] /= this->_calCount[i];
        this->config.outPoints[i] /= this->_calCount[i];
#if PIECEWISE_LINEAR_VALUE_MAPPER_ENABLE_K
        this->config.out_k[i] = (this->config.out_points[i + 1] - this->config.out_points[i]) /
                                (this->config.in_points[i + 1] - this->config.in_points[i]);
#endif
    }
    this->config.inPoints[N - 1] /= this->_calCount[N - 1];
    this->config.outPoints[N - 1] /= this->_calCount[N - 1];
#if PIECEWISE_LINEAR_VALUE_MAPPER_ENABLE_K
    this->config.out_k[N - 1] = (this->config.out_wrap - this->config.out_points[N - 1]) /
                                (this->config.in_wrap - this->config.in_points[N - 1]);
#endif
}
template <size_t N>
u32 PiecewiseLinearValueMapper<N>::_wrap(i32 value) {
    if (value > (i32)this->config.inWrap) {
        value -= (i32)this->config.inWrap;
    } else if (value < 0) {
        value += (i32)this->config.inWrap;
    }
    return value;
}
template <size_t N>
void PiecewiseLinearValueMapper<N>::setZeroOffset(u32 zero_offset) {
    this->config.zeroOffset = zero_offset;
}

template <size_t N>
void PiecewiseLinearValueMapper<N>::calibrate(u8 step, u32 in_value, f32 out_value) {
    auto wrap_value = _wrap((i32)in_value - (i32)this->config.zeroOffset);
    this->config.inPoints[step] += wrap_value;
    this->config.outPoints[step] += out_value;
    this->_calCount[step] += 1;
}

template <size_t N>
void PiecewiseLinearValueMapper<N>::beginCalirate() {
    for (u32 i = 0; i < N; ++i) {
        this->config.inPoints[i]  = 0;
        this->config.outPoints[i] = 0;
#if PIECEWISE_LINEAR_VALUE_MAPPER_ENABLE_K
        this->config.out_k[i] = 0;
#endif
        this->_calCount[i] = 0;
    }
}
template <size_t N>
f32 PiecewiseLinearValueMapper<N>::getValue(u32 raw_value) {
    auto wrap_value = _wrap((i32)raw_value - (i32)this->config.zeroOffset);
    for (size_t i = 0; i < N - 1; ++i) {
        if (wrap_value >= this->config.inPoints[i] && wrap_value <= this->config.inPoints[i + 1]) {
#if PIECEWISE_LINEAR_VALUE_MAPPER_ENABLE_K
            _last_value = this->config.out_points[i] +
                          this->config.out_k[i] * (wrap_value - this->config.in_points[i]);
#else
            _lastValue = this->config.outPoints[i] +
                         (this->config.outPoints[i + 1] - this->config.outPoints[i]) /
                             (this->config.inPoints[i + 1] - this->config.inPoints[i]) *
                             (wrap_value - this->config.inPoints[i]);
#endif
            return _lastValue;
        }
    }
    if (wrap_value >= this->config.inPoints[N - 1]) {
#if PIECEWISE_LINEAR_VALUE_MAPPER_ENABLE_K
        _last_value = this->config.out_points[N - 1] +
                      this->config.out_k[N - 1] * (wrap_value - this->config.in_points[N - 1]);
#else
        _lastValue = this->config.outPoints[N - 1] +
                     (this->config.outWrap - this->config.outPoints[N - 1]) /
                         (this->config.inWrap - this->config.inPoints[N - 1]) *
                         (wrap_value - this->config.inPoints[N - 1]);
#endif
        return _lastValue;
    }
    return NAN;
}

}  // namespace wibot

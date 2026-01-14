#include "velocity-estimator.hpp"
#include "math.hpp"
#include <algorithm>
#include <cmath>

namespace wibot {

void VelocityEstimator::applyConfig() {
    // 配置低通滤波器
    // f_c = 4 / (2π * N * T_s)
    _lpConfig.samplePeriod = _config.samplePeriod;
    _lpConfig.wrapValue    = 0.0f;
    f32 tau                = _config.samplePeriod * _config.trackingCycles / 4.0f;
    _lpConfig.cutoffFreq   = k1_2PI / tau;
    _lpFilter.applyConfig();
    _lpFilter.reset();
}

void VelocityEstimator::reset(f32 speed) {
    _speed = speed;
    _lpFilter.reset();
}

void VelocityEstimator::update(i32 displacement) {
    update(displacement, _config.samplePeriod);
}

void VelocityEstimator::update(i32 displacement, f32 samplePeriod) {
    // 计算原始速度
    f32 rawSpeed = static_cast<f32>(displacement) / samplePeriod;

    // 使用滤波器平滑速度输出
    _speed = _lpFilter.filter(rawSpeed);
}

f32 VelocityEstimator::getAngularSpeed() const {
    // 将速度转换为弧度/s
    // 角速度 = (speed / resolution) * 2π
    f32 rps = _speed / static_cast<f32>(_config.resolution);
    return rps * k2PI;
}

}  // namespace wibot

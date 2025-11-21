#include "offset-calibrator.hpp"

namespace wibot {

OffsetCalibrator::OffsetCalibrator() : _currentSampleCount(0), _accumulator(0), _offset(0) {
}

void OffsetCalibrator::reset() {
    _currentSampleCount = 0;
    _accumulator        = 0;
    _offset             = 0;
}

void OffsetCalibrator::addSample(u16 value) {
    _accumulator += value;
    _currentSampleCount++;
}

bool OffsetCalibrator::calculate() {
    if (_currentSampleCount == 0) {
        return false;
    }

    // 计算平均值作为偏移量
    u32 average = static_cast<u32>(_accumulator / _currentSampleCount);

    // 将偏移设置为负的平均值，这样应用偏移后会将读数校准到0附近
    // 注意：这里需要根据ADC分辨率进行适当的转换
    _offset = -static_cast<i16>(average);

    return true;
}

i16 OffsetCalibrator::getOffset() const {
    return _offset;
}

u16 OffsetCalibrator::getSampleCount() const {
    return _currentSampleCount;
}

}  // namespace wibot
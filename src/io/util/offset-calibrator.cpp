#include "offset-calibrator.hpp"

namespace wibot {

OffsetCalibrator::OffsetCalibrator(Storage& storage) : _storage(storage) {
    reset();
}

void OffsetCalibrator::reset() {
    _storage.currentSampleCount = 0;
    _storage.accumulator        = 0;
    _storage.offset             = 0;
}

void OffsetCalibrator::addSample(u16 value) {
    _storage.accumulator += value;
    _storage.currentSampleCount++;
}

bool OffsetCalibrator::calculate() {
    if (_storage.currentSampleCount == 0) {
        return false;
    }

    // 计算平均值作为偏移量
    u32 average = static_cast<u32>(_storage.accumulator / _storage.currentSampleCount);

    // 将偏移设置为负的平均值，这样应用偏移后会将读数校准到0附近
    // 注意：这里需要根据ADC分辨率进行适当的转换
    _storage.offset = -static_cast<i16>(average);

    return true;
}

i16 OffsetCalibrator::getOffset() const {
    return _storage.offset;
}

u16 OffsetCalibrator::getSampleCount() const {
    return _storage.currentSampleCount;
}

}  // namespace wibot
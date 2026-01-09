#include "dsp/util/offset-calibrator.hpp"

namespace wibot {

void OffsetCalibrator::reset() {
    _currentSampleCount = 0;
    _accumulator        = 0;
    _offset             = 0;
}

void OffsetCalibrator::addSample(u16 value) {
    _accumulator += static_cast<u32>(value);
    ++_currentSampleCount;
}

bool OffsetCalibrator::calculate() {
    if (_currentSampleCount == 0) {
        return false;
    }

    u32 average = _accumulator / _currentSampleCount;
    _offset     = static_cast<i16>(average);
    return true;
}

i16 OffsetCalibrator::getOffset() const {
    return _offset;
}

u16 OffsetCalibrator::getSampleCount() const {
    return _currentSampleCount;
}

}  // namespace wibot

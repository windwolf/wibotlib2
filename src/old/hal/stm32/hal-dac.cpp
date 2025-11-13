#include "hal-dac.hpp"

#ifdef HAL_DAC_MODULE_ENABLED

namespace wibot {

Dac::Dac(DAC_HandleTypeDef& handle) : _handle(handle) {
    Initializer::getInstance().registerInitialObject(this);
};

Dac::~Dac() {
    PeripheralManager::getInstance().unregisterPeripheral(this);
};

void Dac::_init() {
    PeripheralManager::getInstance().registerPeripheral(this, &_handle);
};

//	Result Dac::read(Buffer32 buffer)
//	{
//		return Result::NotSupport;
//	};

Result Dac::setConfig(DacConfig& config) {
    _config = config;
    return Result::kOk;
};

Result Dac::start(DacChannel channel) {
    if (channel & kDacChannel1) {
        HAL_DAC_Start(&_handle, DAC_CHANNEL_1);
    }
    if (channel & kDacChannel2) {
        HAL_DAC_Start(&_handle, DAC_CHANNEL_2);
    }
    return Result::kOk;
};

Result Dac::stop(DacChannel channel) {
    if (channel & kDacChannel1) {
        HAL_DAC_Stop(&_handle, DAC_CHANNEL_1);
    }
    if (channel & kDacChannel2) {
        HAL_DAC_Stop(&_handle, DAC_CHANNEL_2);
    }
    return Result::kOk;
};

Result Dac::setValue(DacChannel channel, f32 value) {
    if (value > 1.0f)
        value = 1.0f;
    else if (value < 0.0f)
        value = 0.0f;
    if (channel == kDacChannel1) {
        HAL_DAC_SetValue(
            &_handle, DAC_CHANNEL_1,
            (_config.alignment == kDacAlignment12) ? DAC_ALIGN_12B_R : DAC_ALIGN_8B_R,
            (_config.alignment == kDacAlignment12) ? (u32)(value * 0xFFF) : (u32)(value * 0xFF));
    } else if (channel == kDacChannel2) {
        HAL_DAC_SetValue(
            &_handle, DAC_CHANNEL_2,
            (_config.alignment == kDacAlignment12) ? DAC_ALIGN_12B_R : DAC_ALIGN_8B_R,
            (_config.alignment == kDacAlignment12) ? (u32)(value * 0xFFF) : (u32)(value * 0xFF));
    } else {
    }
    return Result::kOk;
}
Result Dac::calibrate(DacChannel channel) {
    //HAL_DACEx_SelfCalibrate(&_handle, 1);
    return Result::kOk;
};

}  // namespace wibot

#endif

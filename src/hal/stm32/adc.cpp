
#include "hal/stm32/adc.hpp"

namespace wibot::hal {

AdcRegularSource::AdcRegularSource(ADC_HandleTypeDef& hadc, const Config& config)
    : _ins(&hadc), _config(config), _isRunning(false) {
}

AdcRegularSource::AdcRegularSource(ADC_HandleTypeDef& hadc, u8 adcResolution, bool continuousMode)
    : AdcRegularSource(hadc, Config{adcResolution, continuousMode}) {
}

AdcRegularSource::~AdcRegularSource() {
    // Ensure ADC is stopped before destruction.
    if (_ins && _isRunning) {
        HAL_ADC_Stop_DMA(_ins);
    }
    _isRunning = false;
}

Result AdcRegularSource::start(Slice buffer) {
    if (_isRunning) {
        return Result::kBusy;
    }

    HAL_StatusTypeDef status = HAL_ADC_Start_DMA(_ins, (u32*)buffer.data, buffer.size / 2);

    if (status == HAL_OK) {
        _isRunning = true;
        return Result::kOk;
    }

    return Result(status);
}

Result AdcRegularSource::stop() {
    if (!_isRunning) {
        return Result::kOk;
    }

    HAL_StatusTypeDef status = HAL_ADC_Stop_DMA(_ins);

    if (status == HAL_OK) {
        _isRunning = false;
        return Result::kOk;
    }

    return Result(status);
}

bool AdcRegularSource::isRunning() const {
    return _isRunning;
}

os::AsyncResult AdcRegularSource::triggerSingleConversion(Slice buffer) {
    if (_config.continuousMode) {
        return os::AsyncResult::fromError(Result::kNotSupport);
    }

    HAL_StatusTypeDef status = HAL_ADC_Start_DMA(_ins, (u32*)buffer.data, buffer.size / 2);
    if (status != HAL_OK) {
        return os::AsyncResult::fromError(Result(status));
    }
    return _asyncSource.getResult(false);
}

Result AdcRegularSource::reconfigure(const Config& config) {
    if (_isRunning) {
        return Result::kBusy;
    }

    _config = config;

    Result validateResult = _validateConfig();
    if (!validateResult.isOk()) {
        return validateResult;
    }

    return Result::kOk;
}

Result AdcRegularSource::_validateConfig() const {
    if (_config.adcResolution != 8 && _config.adcResolution != 10 && _config.adcResolution != 12 &&
        _config.adcResolution != 16) {
        return Result::kInvalidParameter;
    }

    return Result::kOk;
}

}  // namespace wibot::hal

#include "adc.hpp"
#ifdef HAL_ADC_MODULE_ENABLED
namespace wibot {

AdcRegularSource::AdcRegularSource(ADC_HandleTypeDef& hadc, const Config& config)
    : _ins(&hadc), _config(config), _isRunning(false) {
    PeripheralManager::getInstance().registerPeripheral(this, _ins);
    HAL_ADC_RegisterCallback(_ins, HAL_ADC_CONVERSION_COMPLETE_CB_ID, onConversionCplt);
    HAL_ADC_RegisterCallback(_ins, HAL_ADC_ERROR_CB_ID, onError);
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

    HAL_ADC_UnRegisterCallback(_ins, HAL_ADC_ERROR_CB_ID);
    HAL_ADC_UnRegisterCallback(_ins, HAL_ADC_CONVERSION_COMPLETE_CB_ID);
    PeripheralManager::getInstance().unregisterPeripheral(this);
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

AsyncResult AdcRegularSource::triggerSingleConversion(Slice buffer) {
    if (_config.continuousMode) {
        return AsyncResult::fromError(Result::kNotSupport);
    }
    HAL_StatusTypeDef status = HAL_ADC_Start_DMA(_ins, (u32*)buffer.data, buffer.size / 2);
    if (status != HAL_OK) {
        return AsyncResult::fromError(Result(status));
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

void AdcRegularSource::onConversionCplt(ADC_HandleTypeDef* instance) {
    auto peripheral =
        static_cast<AdcRegularSource*>(PeripheralManager::getInstance().getPeripheral(instance));
    peripheral->_asyncSource.setDone();
}

void AdcRegularSource::onError(ADC_HandleTypeDef* instance) {
    auto peripheral =
        static_cast<AdcRegularSource*>(PeripheralManager::getInstance().getPeripheral(instance));
    peripheral->_asyncSource.setError(Result::kError);
}

}  // namespace wibot

#endif

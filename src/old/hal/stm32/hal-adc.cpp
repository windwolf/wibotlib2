#include "hal-adc.hpp"

#ifdef HAL_ADC_MODULE_ENABLED

namespace wibot {

Adc::Adc(ADC_HandleTypeDef& handle) : _handle(handle) {
    Initializer::getInstance().registerInitialObject(this);
};

Adc::~Adc() {
    PeripheralManager::getInstance().unregisterPeripheral(this);
};

void Adc::_init() {
    HAL_ADC_RegisterCallback(&_handle, HAL_ADC_CONVERSION_COMPLETE_CB_ID, &Adc::_onConversionCplt);
    HAL_ADC_RegisterCallback(&_handle, HAL_ADC_ERROR_CB_ID, &Adc::_onError);
    PeripheralManager::getInstance().registerPeripheral(this, &_handle);
};

//	Result Adc::read(Buffer32 buffer)
//	{
//		return Result::NotSupport;
//	};

Result Adc::start(Buffer32 buffer, WaitHandler& waitHandler) {
    if (_waitTrigger.isAttached()) {
        return Result::kBusy;
    }
    _buffer = buffer;
    _waitTrigger.attach(waitHandler);
    return (Result)HAL_ADC_Start_DMA(&_handle, buffer.data, buffer.size);
};

Result Adc::stop() {
    _waitTrigger.setDone();
    _waitTrigger.detach();
    return (Result)HAL_ADC_Stop_DMA(&_handle);
};

Result Adc::read(Buffer32 buffer) {
    HAL_ADC_Start(&_handle);
    buffer.data[0] = HAL_ADC_GetValue(&_handle);
    return Result::kOk;
};

void Adc::_onConversionCplt(ADC_HandleTypeDef* instance) {
    Adc* perip = (Adc*)PeripheralManager::getInstance().getPeripheral(instance);

    perip->_waitTrigger.setDone();
    perip->_waitTrigger.detach();
};
void Adc::_onError(ADC_HandleTypeDef* instance) {
    Adc* perip = (Adc*)PeripheralManager::getInstance().getPeripheral(instance);

    perip->_waitTrigger.setError();
    perip->_waitTrigger.detach();
}

}  // namespace wibot

#endif

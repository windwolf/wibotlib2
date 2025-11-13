#include "pin.hpp"

namespace wibot {

Pin::Pin(GPIO_TypeDef* port, u16 pinMask)
    : config({.value = 0}), _port(port), _pinMask(pinMask) {

      };

PinStatus Pin::read() {
    auto curr = static_cast<PinStatus>(HAL_GPIO_ReadPin(_port, this->_pinMask));

    if (config.debounceTime != 0) {
        if (_isFirstValue) {
            _isFirstValue       = false;
            _lastBufferedStatus = curr;
            _lastDebounceTime   = HAL_GetTick();
            _lastOutputStatus =
                static_cast<PinStatus>(toUnderlying(_lastBufferedStatus) ^ this->config.inverse);
            return _lastOutputStatus;
        }
        if (curr != _lastBufferedStatus) {
            _lastBufferedStatus = curr;
            _lastDebounceTime   = HAL_GetTick();

        } else {
            if (HAL_GetTick() - _lastDebounceTime > this->config.debounceTime) {
                _lastOutputStatus = static_cast<PinStatus>(toUnderlying(_lastBufferedStatus) ^
                                                           this->config.inverse);
            }
        }
        return _lastOutputStatus;
    } else {
        return static_cast<PinStatus>(toUnderlying(curr) ^ this->config.inverse);
    }
};

void Pin::write(PinStatus value) {
    HAL_GPIO_WritePin(_port, this->_pinMask,
                      (GPIO_PinState)(toUnderlying(value) ^ this->config.inverse));
};

Result Pin::toggle() {
    HAL_GPIO_TogglePin(_port, this->_pinMask);
    return Result::kOk;
};

Result Pin::setMode(PinMode mode) {
    LL_GPIO_SetPinMode(_port, this->_pinMask,
                       (mode == PinMode::kInput) ? LL_GPIO_MODE_INPUT : LL_GPIO_MODE_OUTPUT);
    return Result::kOk;
};

}  // namespace wibot

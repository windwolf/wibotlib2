#include "gpio.hpp"
#include "system.hpp"

namespace wibot {



// ============================================================================
// Pin 类实现

Pin::Pin(GPIO_TypeDef* port, u16 pin, bool inverse)
    : _pin(GpioPinConfig(port, pin)), _inverse(inverse) {
}

bool Pin::getValue() const {
    return (HAL_GPIO_ReadPin(_pin.port, _pin.pin) == GPIO_PIN_SET) ^ _inverse;
};

void Pin::setValue(bool value) {
    HAL_GPIO_WritePin(_pin.port, _pin.pin, static_cast<GPIO_PinState>(value ^ _inverse));
};

void Pin::setConfig(bool inverse) {
    _inverse = inverse;
};

}  // namespace wibot
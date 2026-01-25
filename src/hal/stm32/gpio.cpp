#include "gpio.hpp"
#include "../system.hpp"

namespace wibot {

// ============================================================================
// Pin  类实现

Pin::Pin(GPIO_TypeDef* port, u16 pin, bool inverse) : _pin(Config(port, pin, inverse)) {
}

bool Pin ::getValue() const {
    return (HAL_GPIO_ReadPin(_pin.port, _pin.pin) == GPIO_PIN_SET) ^ _pin.inverse;
};

void Pin ::setValue(bool value) {
    HAL_GPIO_WritePin(_pin.port, _pin.pin, static_cast<GPIO_PinState>(value ^ _pin.inverse));
};

void Pin ::setConfig(bool inverse) {
    _pin.inverse = inverse;
};

}  // namespace wibot
#include "gpio.hpp"
#include "../system.hpp"

namespace wibot::hal {

// ============================================================================
// hal::Pin  类实现

Pin::Pin(GPIO_TypeDef* port, u16 pin, bool inverse) : _pin(Config(port, pin)), _inverse(inverse) {
}

bool hal::Pin ::getValue() const {
    return (HAL_GPIO_ReadPin(_pin.port, _pin.pin) == GPIO_PIN_SET) ^ _inverse;
};

void hal::Pin ::setValue(bool value) {
    HAL_GPIO_WritePin(_pin.port, _pin.pin, static_cast<GPIO_PinState>(value ^ _inverse));
};

void hal::Pin ::setConfig(bool inverse) {
    _inverse = inverse;
};

}  // namespace wibot::hal
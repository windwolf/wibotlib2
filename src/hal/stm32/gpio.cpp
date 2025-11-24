#include "gpio.hpp"
#include "system.hpp"

namespace wibot {

// ============================================================================
// GpioDigitalSource 模板实现
// ============================================================================

template <u8 CHANNELS>
GpioDigitalSource<CHANNELS>::GpioDigitalSource(const GpioDigitalSourceConfig& config)
    : DigitalSource<CHANNELS>(config.digitalConfig), _config(config){};

template <u8 CHANNELS>
void GpioDigitalSource<CHANNELS>::update() {
    // 从GPIO硬件读取所有通道的值
    u32 rawValues = readAllGpioChannels();

    // 更新原始值到基类
    this->updateRawValues(rawValues);

    // 调用基类的处理逻辑
    DigitalSource<CHANNELS>::update();
};

template <u8 CHANNELS>
u32 GpioDigitalSource<CHANNELS>::readAllGpioChannels() const {
    u32 result = 0;

    // 逐个读取每个通道的GPIO状态
    for (u8 i = 0; i < _config.pinCount && i < CHANNELS; i++) {
        if (_config.pins[i].port != nullptr) {
            GPIO_PinState pinState = HAL_GPIO_ReadPin(_config.pins[i].port, _config.pins[i].pin);
            if (pinState == GPIO_PIN_SET) {
                result |= (1U << i);  // 设置对应位
            }
        }
    }

    return result;
};

template <u8 CHANNELS>
void GpioDigitalSource<CHANNELS>::configureGpio(const GpioDigitalSourceConfig& config) {
    _config = config;

    // 重置基类状态
    this->reset();
};

template <u8 CHANNELS>
bool GpioDigitalSource<CHANNELS>::_isValidChannel(u8 channel) const {
    return channel < CHANNELS;
};

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
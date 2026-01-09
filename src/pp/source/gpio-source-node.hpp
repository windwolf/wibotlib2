#pragma once

#include "../pipeline.hpp"
#include "chip.hpp"
#include "hal/stm32/gpio.hpp"

namespace wibot::pp {

/**
 * @brief GPIO数字输入源节点
 * 
 * 从STM32 GPIO引脚读取数字输入状态，作为pipeline的数据源节点
 * 输出u32位掩码，每位代表一个GPIO通道的状态
 * 
 * 使用示例:
 * @code
 * GpioDigitalSourceNode<2>::GpioPinConfig pins[] = {
 *     {GPIOB, GPIO_PIN_1},  // 通道0: PB1  
 *     {GPIOB, GPIO_PIN_0},  // 通道1: PB0
 * };
 * 
 * GpioDigitalSourceNode<2>::Config config = {
 *     .pins = pins,
 *     .pinCount = 2
 * };
 * 
 * GpioDigitalSourceNode<2> gpioSource(config);
 * 
 * // 在pipeline builder中绑定输出
 * u32 gpioData;
 * builder.bind(gpioSource.outputs.status, gpioData);
 * @endcode
 * 
 * @tparam CHANNELS 通道数量（最多32个通道）
 */
template <u8 CHANNELS>
    requires(CHANNELS <= 32)
class GpioDigitalSourceNode : public INode {
   public:
    /**
     * @brief GPIO数字输入源配置
     */
    struct Config {
        hal::Pin::Config pins[CHANNELS];  ///< GPIO引脚配置数组，最多32个
    };

    struct Outputs {
        Out<u32> status;  ///< 输出状态位掩码
    } outputs;

    /**
     * @brief 构造GPIO数字输入源节点
     * 
     * @param config GPIO引脚配置
     */
    explicit GpioDigitalSourceNode(Config& config) : _config(config) {
    }

    bool ready() override {
        return outputs.status.bound();
    }

    void process() override {
        outputs.status.ref() = readAllGpioChannels();
    }

    void reset() override {
        // GPIO源节点无需重置状态
    }

    /**
     * @brief 读取所有配置的GPIO状态
     * 
     * @return 32位掩码，每位代表一个通道的GPIO状态
     */
    u32 readAllGpioChannels() const {
        u32 result = 0;

        // 逐个读取每个通道的GPIO状态
        for (u8 i = 0; i < CHANNELS; i++) {
            if (_config.pins[i].port != nullptr) {
                GPIO_PinState pinState =
                    HAL_GPIO_ReadPin(_config.pins[i].port, _config.pins[i].pin);
                if (pinState == GPIO_PIN_SET) {
                    result |= (1U << i);  // 设置对应位
                }
            }
        }

        return result;
    }

   private:
    Config& _config;
};

}  // namespace wibot::pp

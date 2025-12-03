#pragma once

#include "chip.hpp"
#include "digital-source.hpp"

namespace wibot {

/**
 * @brief GPIO端口和引脚配置结构
 */
struct GpioPinConfig {
    GPIO_TypeDef* port;  ///< GPIO端口（GPIOA, GPIOB等）
    u16           pin;   ///< GPIO引脚（GPIO_PIN_0, GPIO_PIN_1等）
};

/**
 * @brief GPIO数字输入源配置
 */
struct GpioDigitalSourceConfig {
    DigitalSourceConfig digitalConfig;  ///< 基础数字输入配置（包含按通道的取反掩码）
    GpioPinConfig*      pins;           ///< GPIO引脚配置数组，最多32个
    u8                  pinCount;
};

/**
 * @brief 基于STM32 HAL库的GPIO数字输入源
 * 
 * 继承自DigitalSource，提供实际的GPIO硬件访问功能
 * 支持信号取反功能，与Pin类的inverse概念一致
 * 
 * 使用示例:
 * @code
 * GpioPinConfig pins[] = {
 *     {GPIOB, GPIO_PIN_1},  // 通道0: PB1  
 *     {GPIOB, GPIO_PIN_0},  // 通道1: PB0
 * };
 * 

 * GpioDigitalSourceConfig config = {
 *     .digitalConfig = {.inverse = 0x02, .debounceTimeMs = 50},  // 仅通道1取反，二进制: 10
 *     .pins = pins,
 *     .pinCount = 2
 * };
 * 
 * GpioDigitalSource<2> gpioSource(config);
 * @endcode
 * 
 * @tparam CHANNELS 通道数量（最多32个通道）
 */
template <u8 CHANNELS>
class GpioDigitalSource : public DigitalSource<CHANNELS> {
   public:
    static_assert(CHANNELS <= 32, "CHANNELS must not exceed 32");

   public:
    /**
     * @brief 构造GPIO数字输入源
     * 
     * @param config GPIO数字输入配置参数
     */
    explicit GpioDigitalSource(const GpioDigitalSourceConfig& config)
        : DigitalSource<CHANNELS>(config.digitalConfig), _config(config) {
    }

    /**
     * @brief 更新GPIO输入值
     * 
     * 从实际GPIO端口读取所有配置通道的值，并调用基类的处理逻辑
     */
    void update() override {
        // 从GPIO硬件读取所有通道的值
        u32 rawValues = readAllGpioChannels();

        // 更新原始值到基类
        this->updateRawValues(rawValues);

        // 调用基类的处理逻辑
        DigitalSource<CHANNELS>::update();
    }

    /**
     * @brief 读取所有配置的GPIO状态
     * 
     * @return 32位掩码，每位代表一个通道的GPIO状态
     */
    u32 readAllGpioChannels() const {
        u32 result = 0;

        // 逐个读取每个通道的GPIO状态
        for (u8 i = 0; i < _config.pinCount && i < CHANNELS; i++) {
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

    /**
     * @brief 重新配置GPIO引脚
     * 
     * @param config 新的GPIO数字输入配置
     */
    void configureGpio(const GpioDigitalSourceConfig& config) {
        _config = config;

        // 更新基类配置，包括取反设置
        this->configure(config.digitalConfig);

        // 重置基类状态
        this->reset();
    }

   private:
    /**
     * @brief 验证通道索引有效性
     * 
     * @param channel 通道索引
     * @return 是否有效
     */
    bool _isValidChannel(u8 channel) const {
        return channel < CHANNELS;
    }

   private:
    GpioDigitalSourceConfig _config;
};

/**
 * @brief 单个GPIO引脚控制类
 * 
 * 提供单个GPIO引脚的读写功能，支持信号取反
 * 取反功能与GpioDigitalSource的inverse概念一致
 * 
 * 使用示例:
 * @code
 * // 创建一个取反的引脚（低电平有效）
 * Pin ledPin(GPIOC, GPIO_PIN_13, true);  // inverse=true
 * 
 * // 设置引脚为高逻辑电平（实际输出低电平）
 * ledPin.setValue(true);  // 由于取反，实际GPIO输出为低电平
 * @endcode
 */
class Pin {
   public:
    /**
     * @brief 构造GPIO引脚控制器
     * 
     * @param port GPIO端口（GPIOA, GPIOB等）
     * @param pin GPIO引脚（GPIO_PIN_0, GPIO_PIN_1等）
     * @param inverse 是否取反，true表示逻辑取反
     */
    Pin(GPIO_TypeDef* port, u16 pin, bool inverse = false);

   public:
    /**
     * @brief 读取引脚逻辑值（考虑取反设置）
     * 
     * @return 逻辑值，已根据inverse设置进行处理
     */
    bool getValue() const;

    /**
     * @brief 设置引脚逻辑值（考虑取反设置）
     * 
     * @param value 逻辑值，将根据inverse设置转换为实际GPIO电平
     */
    void setValue(bool value);

    /**
     * @brief 更新取反配置
     * 
     * @param inverse 新的取反设置
     */
    void setConfig(bool inverse);

   private:
    GpioPinConfig _pin;      ///< GPIO引脚配置
    bool          _inverse;  ///< 取反配置参数
};

}  // namespace wibot

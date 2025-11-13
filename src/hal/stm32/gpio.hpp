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
    DigitalSourceConfig digitalConfig;  ///< 基础数字输入配置
    GpioPinConfig*      pins;           ///< GPIO引脚配置数组，最多32个
    u8                  pinCount;
};

/**
 * @brief 基于STM32 HAL库的GPIO数字输入源
 * 
 * 继承自DigitalSource，提供实际的GPIO硬件访问功能
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
    explicit GpioDigitalSource(const GpioDigitalSourceConfig& config);

    /**
     * @brief 更新GPIO输入值
     * 
     * 从实际GPIO端口读取所有配置通道的值，并调用基类的处理逻辑
     */
    void update() override;

    /**
     * @brief 读取所有配置的GPIO状态
     * 
     * @return 32位掩码，每位代表一个通道的GPIO状态
     */
    u32 readAllGpioChannels() const;

    /**
     * @brief 重新配置GPIO引脚
     * 
     * @param config 新的GPIO数字输入配置
     */
    void configureGpio(const GpioDigitalSourceConfig& config);

   private:
    /**
     * @brief 验证通道索引有效性
     * 
     * @param channel 通道索引
     * @return 是否有效
     */
    bool _isValidChannel(u8 channel) const;

   private:
    GpioDigitalSourceConfig _config;
};

class Pin {
   public:
    Pin(GPIO_TypeDef* port, u16 pin, bool inverse = false);

   public:
    // Pipeline接口实现
    bool getValue() const;
    void setValue(bool value);

    void setConfig(bool inverse);

   private:
    GpioPinConfig _pin;      ///< GPIO引脚配置
    bool          _inverse;  ///< 配置参数
};



}  // namespace wibot

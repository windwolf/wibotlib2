#pragma once

#include "chip.hpp"

namespace wibot {

/**
 * @brief 单个GPIO引脚控制类
 * 
 * 提供单个GPIO引脚的读写功能，支持信号取反
 * 取反功能与GpioDigitalSource的inverse概念一致
 * 
 * 使用示例:
 * @code
 * // 创建一个取反的引脚（低电平有效）
 * Pin  ledPin(GPIOC, GPIO_PIN_13, true);  // inverse=true
 * 
 * // 设置引脚为高逻辑电平（实际输出低电平）
 * ledPin.setValue(true);  // 由于取反，实际GPIO输出为低电平
 * @endcode
 */
class Pin {
   public:
    struct Config {
        GPIO_TypeDef* port;             ///< GPIO端口（GPIOA, GPIOB等）
        u16           pin;              ///< GPIO引脚（GPIO_PIN_0, GPIO_PIN_1等）
        bool          inverse = false;  ///< 是否取反，true表示逻辑取反
    };

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
    Config _pin;  ///< GPIO引脚配置
};

}  // namespace wibot

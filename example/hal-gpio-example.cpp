/*
 * HalGpio 使用示例
 * 
 * 这个文件展示了如何使用 HalGpio 类来读取GPIO输入
 */

#include "hal-gpio.hpp"
#include "../../port/chip/chip.hpp"

using namespace wibot;

// 使用示例
void exampleUsage() {
    // 创建一个4通道的GPIO数字输入
    HalGpio<4> gpioInput;
    
    // 配置单个引脚 - 例如配置 GPIOA Pin 0 到通道 0
    gpioInput.configurePin(0, GPIOA, GPIO_PIN_0);
    gpioInput.configurePin(1, GPIOA, GPIO_PIN_1);
    gpioInput.configurePin(2, GPIOB, GPIO_PIN_0);
    gpioInput.configurePin(3, GPIOB, GPIO_PIN_1);
    
    // 或者批量配置
    const GPIO_TypeDef* ports[] = {GPIOA, GPIOA, GPIOB, GPIOB};
    const uint16_t pinMasks[] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_0, GPIO_PIN_1};
    gpioInput.configurePins(ports, pinMasks, 4);
    
    while (true) {
        // 更新所有GPIO状态
        gpioInput.update();
        
        // 读取单个通道状态
        bool channel0State = gpioInput.getValue(0);
        bool channel1State = gpioInput.getValue(1);
        
        // 读取所有通道状态（位掩码形式）
        uint32_t allChannels = gpioInput.getValues();
        
        // 检查特定通道是否被按下
        if (channel0State) {
            // 通道0被激活
        }
        
        if (allChannels & (1U << 2)) {
            // 通道2被激活
        }
        
        // 延时一段时间再次检查
        HAL_Delay(10);
    }
}

/*
 * 使用说明：
 * 
 * 1. HalGpio 继承自 DigitalInput，因此具有防抖功能
 * 2. 默认防抖时间为 50ms，可以在构造函数中修改
 * 3. 支持最多32个通道的GPIO输入
 * 4. 每个通道可以配置不同的GPIO端口和引脚
 * 5. update() 方法需要定期调用以更新状态和处理防抖
 * 6. getValue() 返回单个通道的状态
 * 7. getValues() 返回所有通道的位掩码状态
 */
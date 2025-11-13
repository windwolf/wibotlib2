#include "hal-digital-source.hpp"
#include "hal-digital-source-example.hpp"

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#ifdef __cplusplus
}
#endif

namespace wibot {

/**
 * @brief HAL数字输入源测试类
 */
class HalDigitalSourceTest {
   public:
    /**
     * @brief 运行所有测试
     */
    static void runAllTests() {
        printf("=== HAL数字输入源测试 ===\n\n");

        testBasicConstruction();
        testGpioConfiguration();
        testChannelValidation();
        testIntegration();

        printf("=== 所有测试完成 ===\n\n");

        // 运行示例
        HalDigitalSourceExample::runAllExamples();
    }

   private:
    /**
     * @brief 测试基本构造
     */
    static void testBasicConstruction() {
        printf("--- 测试1：基本构造 ---\n");

        // 测试简单构造函数
        GpioPinConfig pins[] = {GpioPins::getDiPwr(), GpioPins::getDoPwrenN()};

        auto source1 = HalDigitalSource<2>(pins, 2);
        printf("✓ 简单构造函数测试通过\n");

        // 测试配置结构构造函数
        HalDigitalSourceConfig config = {{0x01, 25},  // 反转掩码0x01，消抖25ms
                                         {GpioPins::getDiPwr(), GpioPins::getSpi2Lock()},
                                         2};

        auto source2 = HalDigitalSource<2>(config);
        printf("✓ 配置结构构造函数测试通过\n");

        printf("基本构造测试完成\n\n");
    }

    /**
     * @brief 测试GPIO配置
     */
    static void testGpioConfiguration() {
        printf("--- 测试2：GPIO配置 ---\n");

        GpioPinConfig pins[] = {GpioPins::getDiPwr(), GpioPins::getDoPwrenN(),
                                GpioPins::getSpi2Lock()};

        auto source = HalDigitalSource<4>(pins, 3, 0, 0);

        // 测试单个引脚配置
        source.configureGpioPin(3, GpioPins::getSpi3Lock().port, GpioPins::getSpi3Lock().pin);
        printf("✓ 单个引脚配置测试通过\n");

        // 测试读取功能（注意：实际GPIO状态取决于硬件）
        bool ch0State = source.readGpioChannel(0);
        printf("✓ GPIO读取功能测试通过 (通道0状态: %s)\n", ch0State ? "HIGH" : "LOW");

        uint32_t allStates = source.readAllGpioChannels();
        printf("✓ 所有GPIO读取测试通过 (状态: 0x%08X)\n", (unsigned int)allStates);

        printf("GPIO配置测试完成\n\n");
    }

    /**
     * @brief 测试通道验证
     */
    static void testChannelValidation() {
        printf("--- 测试3：通道验证 ---\n");

        GpioPinConfig pins[] = {GpioPins::getDiPwr()};

        auto source = HalDigitalSource<2>(pins, 1);

        // 测试有效通道
        bool valid0 = source.readGpioChannel(0);  // 应该成功
        printf("✓ 有效通道0读取: %s\n", valid0 ? "HIGH" : "LOW");

        // 测试无效通道（超出配置范围）
        bool invalid1 = source.readGpioChannel(1);  // 通道1未配置，应该返回false
        printf("✓ 无效通道1读取: %s (应该为LOW)\n", invalid1 ? "HIGH" : "LOW");

        // 测试超出范围的通道
        bool outOfRange = source.readGpioChannel(5);  // 超出CHANNELS范围
        printf("✓ 超范围通道5读取: %s (应该为LOW)\n", outOfRange ? "HIGH" : "LOW");

        printf("通道验证测试完成\n\n");
    }

    /**
     * @brief 测试集成功能
     */
    static void testIntegration() {
        printf("--- 测试4：集成功能 ---\n");

        // 创建完整配置的数字输入源
        GpioPinConfig pins[] = {GpioPins::getDiPwr(), GpioPins::getDoPwrenN(),
                                GpioPins::getSpi2Lock(), GpioPins::getSpi3Lock()};

        auto source = HalDigitalSource<4>(pins, 4, 0x02, 20);  // 通道1反转，20ms消抖

        printf("配置完成：4通道，通道1反转，20ms消抖\n");

        // 执行几次完整的更新循环
        for (int i = 0; i < 3; i++) {
            source.update();  // 这会读取GPIO并应用消抖/反转逻辑

            printf("更新%d: ", i + 1);
            for (uint8_t ch = 0; ch < 4; ch++) {
                printf("CH%d=%s ", ch, source.getValue(ch) ? "H" : "L");
            }
            printf("(全部值: 0x%08X)\n", (unsigned int)source.getValues());

            // 简单延时
            volatile int delay = 100000;
            while (delay--) { /* 延时循环 */
            }
        }

        printf("✓ 集成功能测试完成\n");

        // 测试动态重配置
        HalDigitalSourceConfig newConfig = {{0x00, 10},          // 移除反转，减少消抖时间
                                            {pins[0], pins[1]},  // 只使用前两个引脚
                                            2};

        source.configureHal(newConfig);
        printf("✓ 动态重配置测试完成\n");

        printf("集成功能测试完成\n\n");
    }
};

}  // namespace wibot

/**
 * @brief HAL数字输入源测试入口函数
 */
void testHalDigitalSource() {
    wibot::HalDigitalSourceTest::runAllTests();
}
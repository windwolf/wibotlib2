#include "hal-digital-source-example.hpp"
#include "../system.hpp"

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#ifdef __cplusplus
}
#endif

namespace wibot {

// ============================================================================
// HalDigitalSourceExample 实现
// ============================================================================

void HalDigitalSourceExample::runAllExamples() {
    printf("=== HAL数字输入源示例集合 ===\n\n");

    example1_BasicGpioReading();
    printf("\n");

    example2_MultiChannelConfiguration();
    printf("\n");

    example3_DebounceDemo();
    printf("\n");

    example4_InverseDemo();
    printf("\n");

    example5_RealWorldUsage();

    printf("=== 所有示例完成 ===\n");
}

void HalDigitalSourceExample::example1_BasicGpioReading() {
    printf("--- 示例1：基本GPIO读取 ---\n");

    // 创建单通道HAL数字输入源
    GpioPinConfig pins[] = {
        GpioPins::getDiPwr()  // 使用DI_PWR引脚
    };

    auto digitalSource = HalDigitalSource<1>(pins, 1, 0, 0);  // 无反转，无消抖

    printf("配置：通道0 -> DI_PWR引脚 (PB1)\n");
    printf("开始读取GPIO状态...\n");

    for (int i = 0; i < 5; i++) {
        digitalSource.update();

        printf("第%d次读取：", i + 1);
        printf("通道0=%s ", digitalSource.getValue(0) ? "HIGH" : "LOW");
        printf("原始GPIO=%s\n", digitalSource.readGpioChannel(0) ? "HIGH" : "LOW");

        simulateDelay(100);
    }

    printf("基本GPIO读取完成\n");
}

void HalDigitalSourceExample::example2_MultiChannelConfiguration() {
    printf("--- 示例2：多通道GPIO配置 ---\n");

    // 配置4通道GPIO
    GpioPinConfig pins[] = {
        GpioPins::getDiPwr(),     // 通道0: DI_PWR (PB1)
        GpioPins::getDoPwrenN(),  // 通道1: DO_PWREN_N (PB0)
        GpioPins::getSpi2Lock(),  // 通道2: SPI2_LOCK (PC6)
        GpioPins::getSpi3Lock()   // 通道3: SPI3_LOCK (PA15)
    };

    auto digitalSource = HalDigitalSource<4>(pins, 4, 0, 0);

    printf("配置：\n");
    printf("  通道0 -> DI_PWR (PB1)\n");
    printf("  通道1 -> DO_PWREN_N (PB0)\n");
    printf("  通道2 -> SPI2_LOCK (PC6)\n");
    printf("  通道3 -> SPI3_LOCK (PA15)\n\n");

    printf("多通道GPIO状态读取：\n");
    for (int i = 0; i < 3; i++) {
        digitalSource.update();

        printf("第%d次读取：", i + 1);
        printChannelStatus(digitalSource);
        printf("所有通道值=0x%08X\n", (unsigned int)digitalSource.getValues());

        simulateDelay(200);
    }

    printf("多通道配置完成\n");
}

void HalDigitalSourceExample::example3_DebounceDemo() {
    printf("--- 示例3：消抖功能演示 ---\n");

    // 使用DI_PWR引脚，启用50ms消抖
    GpioPinConfig pins[] = {GpioPins::getDiPwr()};

    auto digitalSource = HalDigitalSource<1>(pins, 1, 0, 50);  // 50ms消抖

    printf("配置：通道0 -> DI_PWR引脚，消抖时间=50ms\n");
    printf("消抖功能测试（模拟快速变化）：\n");

    // 模拟消抖场景
    for (int cycle = 0; cycle < 3; cycle++) {
        printf("\\n消抖周期 %d:\n", cycle + 1);

        // 快速更新（模拟抖动）
        for (int i = 0; i < 10; i++) {
            digitalSource.update();

            printf("  快速读取%d: 输出=%s, 原始=%s\n", i + 1,
                   digitalSource.getValue(0) ? "HIGH" : "LOW",
                   digitalSource.readGpioChannel(0) ? "HIGH" : "LOW");

            simulateDelay(10);  // 10ms间隔，小于消抖时间
        }

        // 等待消抖时间
        printf("  等待消抖时间...\n");
        simulateDelay(60);  // 超过50ms消抖时间

        digitalSource.update();
        printf("  消抖后输出: %s\n", digitalSource.getValue(0) ? "HIGH" : "LOW");
    }

    printf("消抖功能演示完成\n");
}

void HalDigitalSourceExample::example4_InverseDemo() {
    printf("--- 示例4：反转功能演示 ---\n");

    // 配置2通道，通道1反转
    GpioPinConfig pins[] = {
        GpioPins::getDiPwr(),    // 通道0：正常
        GpioPins::getDoPwrenN()  // 通道1：反转
    };

    uint32_t inverseMask   = 0x02;  // 仅通道1反转（第1位）
    auto     digitalSource = HalDigitalSource<2>(pins, 2, inverseMask, 0);

    printf("配置：\n");
    printf("  通道0 -> DI_PWR (正常)\n");
    printf("  通道1 -> DO_PWREN_N (反转)\n");
    printf("  反转掩码 = 0x%08X\n\n", (unsigned int)inverseMask);

    printf("反转功能测试：\n");
    for (int i = 0; i < 3; i++) {
        digitalSource.update();

        printf("读取%d: ", i + 1);
        printf("CH0: 原始=%s -> 输出=%s  ", digitalSource.readGpioChannel(0) ? "H" : "L",
               digitalSource.getValue(0) ? "H" : "L");
        printf("CH1: 原始=%s -> 输出=%s\n", digitalSource.readGpioChannel(1) ? "H" : "L",
               digitalSource.getValue(1) ? "H" : "L");

        simulateDelay(150);
    }

    printf("反转功能演示完成\n");
}

void HalDigitalSourceExample::example5_RealWorldUsage() {
    printf("--- 示例5：实际应用场景 ---\n");

    // 模拟实际的工业控制场景
    GpioPinConfig pins[] = {
        GpioPins::getDiPwr(),     // 电源状态输入
        GpioPins::getSpi2Lock(),  // SPI2锁定状态
        GpioPins::getSpi3Lock(),  // SPI3锁定状态
        GpioPins::getDoPwrenN()   // 电源使能反馈
    };

    // 配置：电源使能信号反转，30ms消抖
    uint32_t inverseMask  = 0x08;  // 仅通道3（电源使能）反转
    auto     systemStatus = HalDigitalSource<4>(pins, 4, inverseMask, 30);

    printf("工业控制系统状态监控：\n");
    printf("  通道0: 电源状态 (DI_PWR)\n");
    printf("  通道1: SPI2锁定 (SPI2_LOCK)\n");
    printf("  通道2: SPI3锁定 (SPI3_LOCK)\n");
    printf("  通道3: 电源使能反馈 (DO_PWREN_N, 反转)\n");
    printf("  消抖时间: 30ms\n\n");

    printf("系统状态监控（5次采样）：\n");
    for (int i = 0; i < 5; i++) {
        systemStatus.update();

        printf("采样%d: ", i + 1);
        printf("电源=%s ", systemStatus.getValue(0) ? "ON" : "OFF");
        printf("SPI2=%s ", systemStatus.getValue(1) ? "锁定" : "空闲");
        printf("SPI3=%s ", systemStatus.getValue(2) ? "锁定" : "空闲");
        printf("使能反馈=%s ", systemStatus.getValue(3) ? "有效" : "无效");

        // 系统状态评估
        bool powerOk  = systemStatus.getValue(0);
        bool spisBusy = systemStatus.getValue(1) || systemStatus.getValue(2);
        bool enableOk = systemStatus.getValue(3);

        printf("-> 系统状态: ");
        if (powerOk && enableOk) {
            printf("正常运行");
            if (spisBusy) {
                printf(" (通讯中)");
            }
        } else {
            printf("异常");
        }
        printf("\n");

        simulateDelay(100);
    }

    printf("\\n系统监控完成\n");

    // 演示动态重配置
    printf("\\n演示动态重配置：\n");
    DigitalSourceConfig newConfig = {0x00, 10};  // 移除反转，减少消抖时间
    systemStatus.configure(newConfig);
    printf("已更新配置：移除反转，消抖时间改为10ms\n");

    systemStatus.update();
    printf("重配置后状态: ");
    printChannelStatus(systemStatus);

    printf("实际应用场景演示完成\n");
}

template <uint8_t CHANNELS>
void HalDigitalSourceExample::printChannelStatus(const HalDigitalSource<CHANNELS>& source) {
    for (uint8_t i = 0; i < CHANNELS; i++) {
        printf("CH%d=%s ", i, source.getValue(i) ? "H" : "L");
    }
}

void HalDigitalSourceExample::simulateDelay(uint32_t ms) {
    uint32_t startTime = System::getTickMs();
    while ((System::getTickMs() - startTime) < ms) {
        // 简单的延时循环
    }
}

}  // namespace wibot
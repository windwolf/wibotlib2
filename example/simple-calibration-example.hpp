#pragma once

#include "../model/source/analog-source.hpp"
#include "../model/calibration/offset-calibrator.hpp"
#include <iostream>

namespace wibot {

/**
 * @brief 简化的 AnalogSource 偏移校准示例
 * 
 * 展示如何使用 OffsetCalibrator 来校准 AnalogSource：
 * 1. 用户控制采样时机
 * 2. 校准器计算偏移量
 * 3. 直接应用到 AnalogSource
 */
class SimpleCalibrationExample {
   public:
    /**
     * @brief 基本校准示例
     */
    static void basicExample() {
        std::cout << "=== Simple Calibration Example ===" << std::endl;

        // 1. 创建 ADC 源
        AnalogSource<4>::Config adcConfig{12};  // 12位ADC
        AnalogSource<4>         adcSource(adcConfig);

        // 2. 创建校准器
        OffsetCalibrator<4>::Config calibConfig{100};  // 目标100个样本
        OffsetCalibrator<4>         calibrator(calibConfig);

        // 3. 模拟ADC数据（带偏移）
        u16* buffer = adcSource.getBuffer();
        buffer[0]   = 2048;  // 12位ADC中点 + 偏移
        buffer[1]   = 2050;
        buffer[2]   = 2045;
        buffer[3]   = 2052;

        // 显示校准前的数据
        std::cout << "Before calibration:" << std::endl;
        adcSource.update();
        for (u8 ch = 0; ch < 4; ch++) {
            std::cout << "  Ch" << (int)ch << " = " << adcSource.getValue(ch) << std::endl;
        }

        // 4. 开始校准
        calibrator.startCalibration();
        std::cout << "\nStarting calibration (target: " << calibrator.getTargetSampleCount()
                  << " samples)" << std::endl;

        // 5. 用户控制的采样过程
        for (int i = 0; i < 100; i++) {
            // 模拟实际使用中用户控制的采样时机
            // 可以是按钮按下、定时器触发、特定条件满足等

            // 添加当前ADC读数到校准器
            calibrator.addSample(buffer);

            // 每20次显示进度
            if (i % 20 == 0) {
                std::cout << "Progress: " << calibrator.getProgress() << "%" << std::endl;
            }
        }

        // 6. 校准完成，应用结果
        if (calibrator.isReady()) {
            std::cout << "\nCalibration completed!" << std::endl;

            // 显示计算出的偏移量
            std::cout << "Calculated offsets:" << std::endl;
            for (u8 ch = 0; ch < 4; ch++) {
                std::cout << "  Ch" << (int)ch << " offset = " << calibrator.getOffset(ch)
                          << std::endl;
            }

            // 应用到 ADC 源
            calibrator.applyToAnalogSource(adcSource);
            std::cout << "\nOffset applied to AnalogSource" << std::endl;

            // 显示校准后的数据
            std::cout << "\nAfter calibration:" << std::endl;
            adcSource.update();
            for (u8 ch = 0; ch < 4; ch++) {
                std::cout << "  Ch" << (int)ch << " = " << adcSource.getValue(ch) << std::endl;
            }
        }

        std::cout << std::endl;
    }

    /**
     * @brief 条件控制校准示例
     */
    static void conditionalExample() {
        std::cout << "=== Conditional Calibration Example ===" << std::endl;

        // 创建2通道ADC源和校准器
        AnalogSource<2>::Config adcConfig{16};  // 16位高精度ADC
        AnalogSource<2>         adcSource(adcConfig);

        OffsetCalibrator<2>::Config calibConfig{50};
        OffsetCalibrator<2>         calibrator(calibConfig);

        u16* buffer = adcSource.getBuffer();

        // 模拟系统运行状态
        bool systemIdle         = false;
        bool temperatureStable  = false;
        int  idleCounter        = 0;
        bool calibrationStarted = false;

        std::cout << "Monitoring system conditions for calibration..." << std::endl;

        for (int cycle = 0; cycle < 200; cycle++) {
            // 模拟ADC数据更新
            buffer[0] = 32768 + (cycle % 100 - 50);  // 模拟带偏移的数据
            buffer[1] = 32770 + (cycle % 80 - 40);

            // 模拟系统状态检测
            systemIdle        = (cycle > 50 && cycle < 150);  // 中间100个周期系统空闲
            temperatureStable = (cycle > 30);                 // 30个周期后温度稳定

            if (systemIdle) {
                idleCounter++;
            } else {
                idleCounter = 0;
            }

            // 校准触发条件：系统空闲、温度稳定、空闲时间足够
            bool shouldStartCalibration =
                systemIdle && temperatureStable && (idleCounter > 20) && !calibrationStarted;

            if (shouldStartCalibration) {
                std::cout << "\\nCalibration conditions met at cycle " << cycle << std::endl;
                calibrator.startCalibration();
                calibrationStarted = true;
            }

            // 如果正在校准且条件仍满足，继续采样
            if (calibrator.getState() == OffsetCalibrator<2>::State::Collecting) {
                if (systemIdle && temperatureStable) {
                    calibrator.addSample(buffer);

                    if (calibrator.getSampleCount() % 10 == 0) {
                        std::cout << "Calibration progress: " << calibrator.getProgress() << "%"
                                  << std::endl;
                    }
                } else {
                    // 条件不满足，重置校准
                    std::cout << "Conditions lost, resetting calibration" << std::endl;
                    calibrator.reset();
                    calibrationStarted = false;
                }
            }

            // 校准完成
            if (calibrator.isReady()) {
                std::cout << "\\nCalibration completed at cycle " << cycle << std::endl;
                std::cout << "Offsets: Ch0=" << calibrator.getOffset(0)
                          << ", Ch1=" << calibrator.getOffset(1) << std::endl;

                // 应用校准
                calibrator.applyToAnalogSource(adcSource);
                break;
            }

            // 更新ADC源
            adcSource.update();
        }

        std::cout << std::endl;
    }

    /**
     * @brief 手动偏移设置示例
     */
    static void manualOffsetExample() {
        std::cout << "=== Manual Offset Example ===" << std::endl;

        AnalogSource<3>::Config config{10};  // 10位ADC
        AnalogSource<3>         adcSource(config);

        u16* buffer = adcSource.getBuffer();
        buffer[0]   = 512;  // 10位ADC中点
        buffer[1]   = 520;  // 带偏移
        buffer[2]   = 508;  // 带偏移

        std::cout << "Original values:" << std::endl;
        adcSource.update();
        for (u8 ch = 0; ch < 3; ch++) {
            std::cout << "  Ch" << (int)ch << " = " << adcSource.getValue(ch) << std::endl;
        }

        // 手动设置偏移量
        std::cout << "\\nApplying manual offsets..." << std::endl;
        adcSource.setOffset(0, 0);     // 通道0无偏移
        adcSource.setOffset(1, -520);  // 通道1负偏移
        adcSource.setOffset(2, -508);  // 通道2负偏移

        std::cout << "After manual calibration:" << std::endl;
        adcSource.update();
        for (u8 ch = 0; ch < 3; ch++) {
            std::cout << "  Ch" << (int)ch << " = " << adcSource.getValue(ch)
                      << " (offset: " << adcSource.getOffset(ch) << ")" << std::endl;
        }

        std::cout << std::endl;
    }

    /**
     * @brief 批量偏移设置示例
     */
    static void batchOffsetExample() {
        std::cout << "=== Batch Offset Example ===" << std::endl;

        AnalogSource<4>::Config config{12};
        AnalogSource<4>         adcSource(config);

        // 设置预定义的偏移量
        i16 presetOffsets[4] = {-100, -200, -150, -300};
        adcSource.setOffsets(presetOffsets);

        std::cout << "Applied batch offsets:" << std::endl;
        const i16* offsets = adcSource.getOffsets();
        for (u8 ch = 0; ch < 4; ch++) {
            std::cout << "  Ch" << (int)ch << " offset = " << offsets[ch] << std::endl;
        }

        std::cout << std::endl;
    }

    /**
     * @brief 运行所有示例
     */
    static void runAllExamples() {
        std::cout << "Simple AnalogSource Calibration Examples" << std::endl;
        std::cout << std::string(50, '=') << std::endl;

        basicExample();
        conditionalExample();
        manualOffsetExample();
        batchOffsetExample();

        std::cout << std::string(50, '=') << std::endl;
        std::cout << "All examples completed!" << std::endl;

        std::cout << "\\nKey benefits of simplified design:" << std::endl;
        std::cout << "1. OffsetCalibrator only calculates offsets" << std::endl;
        std::cout << "2. AnalogSource directly stores and applies offsets" << std::endl;
        std::cout << "3. User has full control over calibration timing" << std::endl;
        std::cout << "4. Simple and direct API without complex strategies" << std::endl;
        std::cout << "5. Easy to integrate with existing code" << std::endl;
    }
};

}  // namespace wibot
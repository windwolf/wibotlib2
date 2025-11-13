#pragma once

#include "pid-controller.hpp"

namespace wibot {

/**
 * @brief PID控制器使用示例
 * 
 * 展示如何使用PidController进行控制
 */
class PidControllerExample {
   public:
    /**
     * @brief 创建基本的PID控制器示例
     * 
     * 演示了如何配置和使用PidController
     */
    static void basicUsageExample() {
        // 假设有一个传感器数据源管道 (这里用nullptr代替)
        SyncPipeline<f32, f32*>* sensorPipeline = nullptr;

        // 创建双通道PID控制器管道
        PidController<2> pidController(sensorPipeline);

        // 配置PID参数（所有通道共享）
        PidControllerConfig config;
        config.mode       = PidControllerMode::kParallel;
        config.Kp         = 2.0f;   // 比例增益
        config.Ki         = 0.5f;   // 积分增益
        config.Kd         = 0.1f;   // 微分增益
        config.tau        = 0.02f;  // 微分滤波时间常数
        config.sampleTime = 0.01f;  // 10ms采样时间
        config.setPoint   = 50.0f;  // 目标值（所有通道共享）

        // 启用输出限制
        config.outputLimitEnable = true;
        config.outputLimitMax    = 100.0f;
        config.outputLimitMin    = -100.0f;

        // 启用积分限制（防止积分饱和）
        config.integratorLimitEnable = true;
        config.integratorLimitMax    = 50.0f;
        config.integratorLimitMin    = -50.0f;

        pidController.setConfig(config);

        // 在控制循环中使用
        for (int i = 0; i < 1000; ++i) {
            // 更新PID控制器（会自动从上游获取所有通道的测量值）
            pidController.update();

            // 获取单个通道的控制输出
            f32 controlOutput0 = pidController.getValue(0);  // 通道0
            f32 controlOutput1 = pidController.getValue(1);  // 通道1

            // 或者获取所有通道的控制输出
            f32* allOutputs = pidController.getValues();

            // 将控制输出应用到执行器
            // applyToActuator(0, controlOutput0);
            // applyToActuator(1, controlOutput1);
            (void)controlOutput0;  // 避免未使用变量警告
            (void)controlOutput1;
            (void)allOutputs;
        }
    }

    /**
     * @brief 串行模式PID控制器示例
     */
    static void serialModeExample() {
        PidController<1> pidController;  // 单通道

        // 配置为串行模式
        PidControllerConfig config;
        config.mode       = PidControllerMode::kSerial;
        config.Kp         = 1.0f;
        config.Ki         = 0.2f;
        config.Kd         = 0.05f;
        config.tau        = 0.01f;
        config.sampleTime = 0.01f;
        config.setPoint   = 25.0f;

        pidController.setConfig(config);
    }

    /**
     * @brief 动态调整设定值示例
     */
    static void dynamicSetPointExample() {
        PidController<3> pidController;  // 三通道

        // 初始配置
        PidControllerConfig config;
        config.mode       = PidControllerMode::kParallel;
        config.Kp         = 1.5f;
        config.Ki         = 0.3f;
        config.Kd         = 0.08f;
        config.sampleTime = 0.01f;
        config.setPoint   = 0.0f;  // 初始设定值

        pidController.setConfig(config);

        // 控制循环中动态调整设定值
        for (int i = 0; i < 2000; ++i) {
            // 每200次迭代改变设定值
            if (i % 200 == 0) {
                f32 newSetPoint = (i / 200.0f) * 10.0f;
                pidController.setSetPoint(newSetPoint);
            }

            pidController.update();

            // 获取所有通道的输出
            f32* outputs = pidController.getValues();

            // 处理控制输出...
            (void)outputs;  // 避免未使用变量警告
        }
    }

    /**
     * @brief 重置控制器状态示例
     */
    static void resetExample() {
        PidController<4> pidController;  // 四通道

        // 配置控制器
        PidControllerConfig config;
        config.Kp         = 2.0f;
        config.Ki         = 0.5f;
        config.Kd         = 0.1f;
        config.sampleTime = 0.01f;
        config.setPoint   = 30.0f;
        pidController.setConfig(config);

        // 运行一段时间
        for (int i = 0; i < 100; ++i) {
            pidController.update();
        }

        // 重置控制器（清除积分项、微分项等内部状态）
        pidController.reset();

        // 从干净状态重新开始
        for (int i = 0; i < 100; ++i) {
            pidController.update();
        }
    }

    /**
     * @brief 多通道独立控制示例
     */
    static void multiChannelExample() {
        // 使用类型别名创建四通道控制器
        PidController4Ch pidController;

        // 配置共享参数
        PidControllerConfig config;
        config.mode       = PidControllerMode::kParallel;
        config.Kp         = 1.0f;
        config.Ki         = 0.1f;
        config.Kd         = 0.05f;
        config.sampleTime = 0.01f;
        config.setPoint   = 100.0f;  // 所有通道共享设定值

        pidController.setConfig(config);

        // 控制循环
        for (int i = 0; i < 1000; ++i) {
            pidController.update();

            // 分别处理每个通道的输出
            for (u8 ch = 0; ch < 4; ++ch) {
                f32 output = pidController.getValue(ch);
                // applyToActuator(ch, output);
                (void)output;
            }
        }
    }
};

}  // namespace wibot
#pragma once

#include "os/async.hpp"
#include "chip.hpp"
#include "../system.hpp"
#include <cstring>

namespace wibot::hal {

/**
 * @brief 基于STM32 HAL库的ADC数据源（纯DMA实现）
 * 
 * 继承自AnalogSource，仅通过DMA方式获取多通道模拟数据
 * 提供高效的无CPU干预数据采集
 * 
 * @tparam CHANNELS ADC通道数量（最多16个通道）
 */
class AdcRegularSource {
   public:
    /**
     * @brief ADC配置结构
     */
    struct Config {
        u8   adcResolution;   ///< ADC分辨率位数 (8, 10, 12, 16)
        bool continuousMode;  ///< 是否使用连续转换模式
    };

   public:
    /**
     * @brief 构造ADC数据源
     * 
     * @param config ADC配置参数
     */
    explicit AdcRegularSource(ADC_HandleTypeDef& hadc, const Config& config);

    /**
     * @brief 构造ADC数据源（简化版本）
     * 
     * @param hadc ADC句柄指针
     * @param adcResolution ADC分辨率位数，默认12位
     * @param continuousMode 是否使用连续转换模式，默认启用
     */
    explicit AdcRegularSource(ADC_HandleTypeDef& hadc, u8 adcResolution = 12,
                              bool continuousMode = true);

    /**
     * @brief 析构函数
     */
    ~AdcRegularSource();

    /**
     * @brief 启动ADC转换
     * 
     * @return Result::kOk 启动成功，其他值表示失败
     */
    Result start(Slice buffer);

    /**
     * @brief 停止ADC转换
     * 
     * @return Result::kOk 停止成功，其他值表示失败
     */
    Result stop();

    /**
     * @brief 获取ADC是否正在运行
     * 
     * @return true 正在运行，false 已停止
     */
    bool isRunning() const;

    /**
     * @brief 手动触发单次DMA转换（仅在非连续模式下有效）
     * 
     * @return Result::kOk 触发成功，其他值表示失败
     */
    os::AsyncResult triggerSingleConversion(Slice buffer);

    /**
     * @brief 重新配置ADC
     * 
     * @param config 新的ADC配置
     * @return Result::kOk 配置成功，其他值表示失败
     */
    Result reconfigure(const Config& config);

   private:
    /**
     * @brief 验证配置参数
     * 
     * @return Result::kOk 配置有效，其他值表示配置无效
     */
    Result _validateConfig() const;

   private:
    ADC_HandleTypeDef* _ins;
    Config             _config;     ///< ADC配置
    bool               _isRunning;  ///< ADC运行状态
    os::AsyncSource    _asyncSource;

   private:
    static constexpr u8 MAX_CHANNELS = 16;  ///< 最大支持通道数
};

}  // namespace wibot::hal
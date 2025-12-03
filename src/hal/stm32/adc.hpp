#pragma once

#include "async.hpp"
#include "chip.hpp"
#include "analog-source.hpp"
#include "../system.hpp"
#include <cstring>

namespace wibot {

/**
 * @brief ADC配置结构
 */
struct AdcRegularSourceConfig {
    u8   adcResolution;   ///< ADC分辨率位数 (8, 10, 12, 16)
    bool continuousMode;  ///< 是否使用连续转换模式
};

/**
 * @brief 基于STM32 HAL库的ADC数据源（纯DMA实现）
 * 
 * 继承自AnalogSource，仅通过DMA方式获取多通道模拟数据
 * 提供高效的无CPU干预数据采集
 * 
 * @tparam CHANNELS ADC通道数量（最多16个通道）
 */
template <u8 CHANNELS>
class AdcRegularSource : public AnalogSource<CHANNELS> {
   public:
    /**
     * @brief 构造ADC数据源
     * 
     * @param config ADC配置参数
     */
    explicit AdcRegularSource(ADC_HandleTypeDef& hadc, const AdcRegularSourceConfig& config)
        : AnalogSource<CHANNELS>({.resolution = config.adcResolution}),
          _ins(&hadc),
          _config(config),
          _isRunning(false) {
        static_assert(CHANNELS <= MAX_CHANNELS, "CHANNELS must not exceed 16");
    }

    /**
     * @brief 构造ADC数据源（简化版本）
     * 
     * @param hadc ADC句柄指针
     * @param adcResolution ADC分辨率位数，默认12位
     * @param continuousMode 是否使用连续转换模式，默认启用
     */
    explicit AdcRegularSource(ADC_HandleTypeDef& hadc, u8 adcResolution = 12,
                              bool continuousMode = true)
        : AdcRegularSource<CHANNELS>(hadc, AdcRegularSourceConfig{adcResolution, continuousMode}) {
    }

    /**
     * @brief 析构函数
     */
    ~AdcRegularSource() {
        // 确保ADC已停止
        if (_ins && _isRunning) {
            HAL_ADC_Stop_DMA(_ins);
        }
        _isRunning = false;
    }

    /**
     * @brief 启动ADC转换
     * 
     * @return Result::kOk 启动成功，其他值表示失败
     */
    Result start() {
        if (_isRunning) {
            return Result::kBusy;  // 已在运行
        }

        // 始终使用DMA模式启动ADC
        HAL_StatusTypeDef status = HAL_ADC_Start_DMA(_ins, (u32*)this->getBuffer(), CHANNELS);

        if (status == HAL_OK) {
            _isRunning = true;
            return Result::kOk;
        }

        return Result(status);
    }

    /**
     * @brief 停止ADC转换
     * 
     * @return Result::kOk 停止成功，其他值表示失败
     */
    Result stop() {
        if (!_isRunning) {
            return Result::kOk;  // 已经停止
        }

        // 始终使用DMA模式停止ADC
        HAL_StatusTypeDef status = HAL_ADC_Stop_DMA(_ins);

        if (status == HAL_OK) {
            _isRunning = false;
            return Result::kOk;
        }

        return Result(status);
    }

    /**
     * @brief 获取ADC是否正在运行
     * 
     * @return true 正在运行，false 已停止
     */
    bool isRunning() const {
        return _isRunning;
    }

    /**
     * @brief 手动触发单次DMA转换（仅在非连续模式下有效）
     * 
     * @return Result::kOk 触发成功，其他值表示失败
     */
    AsyncResult triggerSingleConversion() {
        if (_config.continuousMode) {
            return AsyncResult::fromError(Result::kNotSupport);  // 处于连续模式，不支持单次转换
        }

        // 始终使用DMA模式进行单次转换
        HAL_StatusTypeDef status = HAL_ADC_Start_DMA(_ins, (u32*)this->getBuffer(), CHANNELS);
        if (status != HAL_OK) {
            return AsyncResult::fromError(Result(status));
        }
        return _asyncSource.getResult(false);
    }

    /**
     * @brief 重新配置ADC
     * 
     * @param config 新的ADC配置
     * @return Result::kOk 配置成功，其他值表示失败
     */
    Result reconfigure(const AdcRegularSourceConfig& config) {
        // 停止当前操作
        bool   wasRunning = _isRunning;
        Result stopResult = stop();
        if (!stopResult.isOk()) {
            return stopResult;
        }

        // 应用新配置
        _config = config;

        // 验证配置
        Result validateResult = _validateConfig();
        if (!validateResult.isOk()) {
            return validateResult;
        }

        // 如果之前在运行，重新启动
        if (wasRunning) {
            return start();
        }

        return Result::kOk;
    }

   private:
    /**
     * @brief 验证配置参数
     * 
     * @return Result::kOk 配置有效，其他值表示配置无效
     */
    Result _validateConfig() const {
        // 检查ADC分辨率
        if (_config.adcResolution != 8 && _config.adcResolution != 10 &&
            _config.adcResolution != 12 && _config.adcResolution != 16) {
            return Result::kInvalidParameter;
        }

        // 检查通道数量
        if (CHANNELS == 0 || CHANNELS > MAX_CHANNELS) {
            return Result::kInvalidParameter;
        }

        return Result::kOk;
    }

   private:
    ADC_HandleTypeDef*     _ins;
    AdcRegularSourceConfig _config;     ///< ADC配置
    bool                   _isRunning;  ///< ADC运行状态
    AsyncSource            _asyncSource;

   private:
    static constexpr u8 MAX_CHANNELS = 16;  ///< 最大支持通道数
};

}  // namespace wibot
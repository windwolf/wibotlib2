#pragma once

#include "type.hpp"

#ifndef DSP_MEDIAN_MAX_WINDOW
#define DSP_MEDIAN_MAX_WINDOW 32
#endif

namespace wibot {

/**
 * @brief 中值滤波器
 * 
 * 面向对象实现。
 */
template <typename T>
class Median {
   public:
    struct Config {
        u8 windowSize{1};  // 滤波窗口大小 (1-32)
    };

    /**
     * @brief 构造函数
     * @param cfg 滤波配置
     */
    explicit Median(const Config& cfg) : _config(cfg), _outputLast(static_cast<T>(0)) {
        ASSERT(isConfigValid(_config), "Invalid Median config");
        _bufferIndex = 0;
        _bufferCount = 0;
        for (u8 i = 0; i < DSP_MEDIAN_MAX_WINDOW; ++i) {
            _buffer[i] = static_cast<T>(0);
        }
    }

    /**
     * @brief 处理单个样本
     * @return 中值滤波后的输出
     */
    T filter(T input) {
        // 添加到环形缓冲区
        _buffer[_bufferIndex] = input;
        _bufferIndex          = (_bufferIndex + 1) % _config.windowSize;

        if (_bufferCount < _config.windowSize) {
            _bufferCount++;
        }

        // 计算中值
        _outputLast = _calculateMedian();
        return _outputLast;
    }

    /**
     * @brief 重置状态
     */
    void reset() {
        _bufferIndex = 0;
        _bufferCount = 0;
        _outputLast  = static_cast<T>(0);
        for (u8 i = 0; i < DSP_MEDIAN_MAX_WINDOW; ++i) {
            _buffer[i] = static_cast<T>(0);
        }
    }

    /**
     * @brief 验证配置有效性
     */
    static bool isConfigValid(const Config& cfg) {
        return cfg.windowSize > 0 && cfg.windowSize <= DSP_MEDIAN_MAX_WINDOW;
    }

   private:
    T _calculateMedian() {
        if (_bufferCount == 0) {
            return static_cast<T>(0);
        }

        // 复制有效数据到临时缓冲区
        T tempBuffer[DSP_MEDIAN_MAX_WINDOW];
        for (u8 i = 0; i < _bufferCount; ++i) {
            tempBuffer[i] = _buffer[i];
        }

        // 使用快速选择找中值
        u8 mid = _bufferCount / 2;

        if (_bufferCount % 2 == 1) {
            // 奇数
            return _quickSelect(tempBuffer, 0, _bufferCount - 1, mid);
        } else {
            // 偶数：返回中间两个值的平均
            T val1 = _quickSelect(tempBuffer, 0, _bufferCount - 1, mid - 1);
            T val2 = _quickSelect(tempBuffer, 0, _bufferCount - 1, mid);
            return static_cast<T>((static_cast<f64>(val1) + static_cast<f64>(val2)) * 0.5);
        }
    }

    static T _quickSelect(T* arr, int left, int right, int k) {
        if (left == right) {
            return arr[left];
        }

        int pivotIndex = _partition(arr, left, right);

        if (k == pivotIndex) {
            return arr[k];
        } else if (k < pivotIndex) {
            return _quickSelect(arr, left, pivotIndex - 1, k);
        } else {
            return _quickSelect(arr, pivotIndex + 1, right, k);
        }
    }

    static int _partition(T* arr, int left, int right) {
        T   pivot = arr[right];
        int i     = left;

        for (int j = left; j < right; ++j) {
            if (arr[j] <= pivot) {
                T temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
                i++;
            }
        }

        T temp     = arr[i];
        arr[i]     = arr[right];
        arr[right] = temp;

        return i;
    }

   private:
    const Config& _config;
    T             _buffer[DSP_MEDIAN_MAX_WINDOW];
    u8            _bufferIndex;
    u8            _bufferCount;
    T             _outputLast;
};

} // namespace wibot


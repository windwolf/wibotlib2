#pragma once

#include <type_traits>
#include "../model.hpp"

#ifndef MEDIAN_FILTER_MAX_WINDOW_SIZE
#define MEDIAN_FILTER_MAX_WINDOW_SIZE 32
#endif

namespace wibot {

/**
 * @brief 中值滤波器管道
 * 
 * 将float输入值进行中值滤波处理。
 * 
 * 中值滤波器通过维护一个滑动窗口，计算窗口内数据的中值：
 * - 对于奇数窗口大小，返回排序后中间位置的值
 * - 对于偶数窗口大小，返回排序后中间两个值的平均值
 * 
 * 中值滤波器特别适用于去除脉冲噪声，能够很好地保持信号的边缘特性。
 * 
 * 配置使用引用方式，支持多个滤波器实例共享同一配置。
 * 内部管理固定大小缓冲区(最大32个样本)。
 */
template <typename T>
class MedianFilter : public SyncPipeline<T> {
    static_assert(std::is_arithmetic<T>(), "T must be an arithmetic type");

   public:
    struct Storage {
        T  buffer[MEDIAN_FILTER_MAX_WINDOW_SIZE]{};
        T  tempBuffer[MEDIAN_FILTER_MAX_WINDOW_SIZE]{};
        u8 bufferIndex{0};
        u8 bufferCount{0};
        T  outputLast{static_cast<T>(0)};
    };

    /**
     * @brief 中值滤波器配置
     */
    struct Config {
        u8 windowSize;  ///< 滤波窗口大小，建议使用奇数 (1-32)
    };

   public:
    /**
     * @brief 构造中值滤波器
     * 
     * @param upstream 上游管道
     * @param config 滤波配置（引用方式，支持共享）
     */
    MedianFilter(SyncPipeline<T>& upstream, const Config& config, Storage& storage)
        : _upstream(upstream), _config(config), _storage(storage) {
    }

    T getValue() const override {
        return _storage.outputLast;
    }

    void reset() override {
        _storage.bufferIndex = 0;
        _storage.bufferCount = 0;
        _storage.outputLast  = static_cast<T>(0);
        for (u8 i = 0; i < MAX_WINDOW_SIZE; ++i) {
            _storage.buffer[i] = static_cast<T>(0);
        }
        _upstream.reset();
    }

    void update() override {
        _upstream.update();

        // 获取新输入
        T input = _upstream.getValue();

        // 添加到环形缓冲区
        _storage.buffer[_storage.bufferIndex] = input;
        _storage.bufferIndex                  = (_storage.bufferIndex + 1) % _config.windowSize;

        if (_storage.bufferCount < _config.windowSize) {
            _storage.bufferCount++;
        }

        // 计算中值
        _storage.outputLast = _calculateMedian();
    }

    static bool isConfigValid(const Config& config) {
        return config.windowSize > 0 && config.windowSize <= MEDIAN_FILTER_MAX_WINDOW_SIZE;
    }

   private:
    T _calculateMedian() {
        if (_storage.bufferCount == 0) return 0.0f;

        // 复制有效数据到临时缓冲区
        for (u8 i = 0; i < _storage.bufferCount; ++i) {
            _storage.tempBuffer[i] = _storage.buffer[i];
        }

        // 使用快速选择算法找中值
        u8 mid = _storage.bufferCount / 2;

        if (_storage.bufferCount % 2 == 1) {
            // 奇数个元素，返回中间值
            return _quickSelect(_storage.tempBuffer, 0, _storage.bufferCount - 1, mid);
        } else {
            // 偶数个元素，返回中间两个值的平均
            T val1 = _quickSelect(_storage.tempBuffer, 0, _storage.bufferCount - 1, mid - 1);
            T val2 = _quickSelect(_storage.tempBuffer, 0, _storage.bufferCount - 1, mid);
            return (val1 + val2) / 2.0f;
        }
    }

    T _quickSelect(T* arr, int left, int right, int k) {
        if (left == right) return arr[left];

        int pivotIndex = _partition(arr, left, right);

        if (k == pivotIndex) {
            return arr[k];
        } else if (k < pivotIndex) {
            return _quickSelect(arr, left, pivotIndex - 1, k);
        } else {
            return _quickSelect(arr, pivotIndex + 1, right, k);
        }
    }

    int _partition(T* arr, int left, int right) {
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
    SyncPipeline<T>& _upstream;  ///< 上游管道引用
    const Config&    _config;    ///< 滤波配置引用（支持共享）
    Storage&         _storage;   ///< 外部存储
};

}  // namespace wibot

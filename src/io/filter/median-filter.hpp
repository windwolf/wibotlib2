#pragma once

#include <type_traits>
#include "../model.hpp"

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
    /**
     * @brief 中值滤波器配置
     */
    struct Config {
        u8 windowSize;  ///< 滤波窗口大小，建议使用奇数 (1-32)
    };

    static constexpr u8 MAX_WINDOW_SIZE = 32;  ///< 最大窗口大小

   public:
    /**
     * @brief 构造中值滤波器
     * 
     * @param upstream 上游管道
     * @param config 滤波配置（引用方式，支持共享）
     */
    MedianFilter(SyncPipeline<T>& upstream, const Config& config)
        : _upstream(upstream),
          _config(config),
          _bufferIndex(0),
          _bufferCount(0),
          _outputLast(static_cast<T>(0)) {
        // 初始化缓冲区
        for (u8 i = 0; i < MAX_WINDOW_SIZE; ++i) {
            _buffer[i]     = static_cast<T>(0);
            _tempBuffer[i] = static_cast<T>(0);
        }
    }

    T getValue() const override {
        return _outputLast;
    }

    void reset() override {
        _bufferIndex = 0;
        _bufferCount = 0;
        _outputLast  = static_cast<T>(0);
        for (u8 i = 0; i < MAX_WINDOW_SIZE; ++i) {
            _buffer[i] = 0.0f;
        }
        _upstream.reset();
    }

    void update() override {
        _upstream.update();

        // 获取新输入
        T input = _upstream.getValue();

        // 添加到环形缓冲区
        _buffer[_bufferIndex] = input;
        _bufferIndex          = (_bufferIndex + 1) % _config.windowSize;

        if (_bufferCount < _config.windowSize) {
            _bufferCount++;
        }

        // 计算中值
        _outputLast = _calculateMedian();
    }

    static bool isConfigValid(const Config& config) {
        return config.windowSize > 0 && config.windowSize <= MAX_WINDOW_SIZE;
    }

   private:
    T _calculateMedian() {
        if (_bufferCount == 0) return 0.0f;

        // 复制有效数据到临时缓冲区
        for (u8 i = 0; i < _bufferCount; ++i) {
            _tempBuffer[i] = _buffer[i];
        }

        // 使用快速选择算法找中值
        u8 mid = _bufferCount / 2;

        if (_bufferCount % 2 == 1) {
            // 奇数个元素，返回中间值
            return _quickSelect(_tempBuffer, 0, _bufferCount - 1, mid);
        } else {
            // 偶数个元素，返回中间两个值的平均
            T val1 = _quickSelect(_tempBuffer, 0, _bufferCount - 1, mid - 1);
            T val2 = _quickSelect(_tempBuffer, 0, _bufferCount - 1, mid);
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

    T  _buffer[MAX_WINDOW_SIZE];      ///< 环形缓冲区
    T  _tempBuffer[MAX_WINDOW_SIZE];  ///< 临时缓冲区(用于排序)
    u8 _bufferIndex;                  ///< 缓冲区当前索引
    u8 _bufferCount;                  ///< 缓冲区有效数据数量
    T  _outputLast;                   ///< 上次的输出值
};

}  // namespace wibot

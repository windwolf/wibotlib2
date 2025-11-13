#include "median-filter.hpp"
#include <algorithm>
#include <cstdlib>

namespace wibot {

// ============================================================================
// MedianFilter 模板实现
// ============================================================================

template <u8 CHANNELS>
MedianFilter<CHANNELS>::MedianFilter(SyncPipeline<f32>& upstream, const Config& config,
                                     f32* buffers[CHANNELS], f32* tempBuffer)
    : _upstream(upstream), _config(config), _tempBuffer(tempBuffer) {
    // 验证配置有效性
    if (!isConfigValid(config)) {
        // 如果配置无效，使用默认配置
        _config.windowSize    = 5;   // 默认窗口大小为5
        _config.maxWindowSize = 32;  // 默认最大窗口大小
    }

    // 设置外部缓冲区指针
    for (u8 i = 0; i < CHANNELS; i++) {
        _buffers[i] = buffers[i];
    }

    // 初始化缓冲区状态
    _initializeBuffers();
}

template <u8 CHANNELS>
f32 MedianFilter<CHANNELS>::getValue(u8 channel) const {
    if (channel >= CHANNELS) {
        return 0.0f;  // 无效通道返回0
    }

    return _outputLast[channel];
}

template <u8 CHANNELS>
void MedianFilter<CHANNELS>::reset() {
    // 重置各通道的滤波状态
    for (u8 i = 0; i < CHANNELS; i++) {
        _bufferIndex[i] = 0;
        _bufferCount[i] = 0;
        _outputLast[i]  = 0.0f;

        // 清空缓冲区
        for (u8 j = 0; j < _config.windowSize; j++) {
            _buffers[i][j] = 0.0f;
        }
    }

    // 重置上游管道
    _upstream.reset();
}

template <u8 CHANNELS>
void MedianFilter<CHANNELS>::update() {
    // 更新上游管道
    _upstream.update();

    // 对所有通道进行滤波处理
    for (u8 i = 0; i < CHANNELS; i++) {
        f32 input = _upstream.getValue(i);

        // 将新值添加到环形缓冲区
        _buffers[i][_bufferIndex[i]] = input;
        _bufferIndex[i]              = (_bufferIndex[i] + 1) % _config.windowSize;

        // 更新有效数据计数
        if (_bufferCount[i] < _config.windowSize) {
            _bufferCount[i]++;
        }

        // 计算中值
        _outputLast[i] = _calculateMedian(i);
    }
}

template <u8 CHANNELS>
void MedianFilter<CHANNELS>::updateConfig(const Config& config) {
    if (isConfigValid(config)) {
        // 更新配置
        _config = config;

        // 重新初始化缓冲区状态
        _initializeBuffers();
    }
    // 如果配置无效，保持原有配置不变
}

template <u8 CHANNELS>
bool MedianFilter<CHANNELS>::isConfigValid(const Config& config) {
    // 检查窗口大小是否有效
    if (config.windowSize == 0 || config.windowSize > config.maxWindowSize) {
        return false;
    }

    // 检查最大窗口大小是否合理
    if (config.maxWindowSize == 0 || config.maxWindowSize > 255) {
        return false;
    }

    return true;
}

template <u8 CHANNELS>
void MedianFilter<CHANNELS>::_initializeBuffers() {
    // 初始化各通道状态和缓冲区
    for (u8 i = 0; i < CHANNELS; i++) {
        _bufferIndex[i] = 0;
        _bufferCount[i] = 0;
        _outputLast[i]  = 0.0f;

        // 初始化缓冲区（使用外部提供的缓冲区）
        for (u8 j = 0; j < _config.maxWindowSize; j++) {
            _buffers[i][j] = 0.0f;
        }
    }

    // 初始化临时缓冲区（使用外部提供的缓冲区）
    for (u8 i = 0; i < _config.maxWindowSize; i++) {
        _tempBuffer[i] = 0.0f;
    }
}

template <u8 CHANNELS>
f32 MedianFilter<CHANNELS>::_calculateMedian(u8 channel) {
    if (_bufferCount[channel] == 0) {
        return 0.0f;
    }

    // 将有效数据复制到临时缓冲区
    for (u8 i = 0; i < _bufferCount[channel]; i++) {
        _tempBuffer[i] = _buffers[channel][i];
    }

    // 对于只有一个数据点的情况
    if (_bufferCount[channel] == 1) {
        return _tempBuffer[0];
    }

    // 使用快速选择算法找到中值
    u8 n = _bufferCount[channel];
    if (n % 2 == 1) {
        // 奇数个元素：返回中间位置的值
        return _quickSelect(_tempBuffer, 0, n - 1, n / 2);
    } else {
        // 偶数个元素：返回中间两个值的平均值
        f32 median1 = _quickSelect(_tempBuffer, 0, n - 1, n / 2 - 1);
        f32 median2 = _quickSelect(_tempBuffer, 0, n - 1, n / 2);
        return (median1 + median2) / 2.0f;
    }
}

template <u8 CHANNELS>
f32 MedianFilter<CHANNELS>::_quickSelect(f32* arr, int left, int right, int k) {
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

template <u8 CHANNELS>
int MedianFilter<CHANNELS>::_partition(f32* arr, int left, int right) {
    f32 pivot = arr[right];  // 选择最右边的元素作为基准
    int i     = left - 1;

    for (int j = left; j < right; j++) {
        if (arr[j] <= pivot) {
            i++;
            // 交换元素
            f32 temp = arr[i];
            arr[i]   = arr[j];
            arr[j]   = temp;
        }
    }

    // 将基准元素放到正确位置
    f32 temp   = arr[i + 1];
    arr[i + 1] = arr[right];
    arr[right] = temp;

    return i + 1;
}

}  // namespace wibot
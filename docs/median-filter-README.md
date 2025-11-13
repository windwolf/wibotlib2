# 中值滤波器使用指南

## 概述

本项目实现了一个仿造`LowpassFilter`的中值滤波器`MedianFilter`，用于多通道数据的中值滤波处理。中值滤波器特别适用于去除脉冲噪声和毛刺，能够很好地保持信号的边缘特性。

## 文件结构

```
wibotlib/src/SyncPipeline/filter/
├── median-filter.hpp          # 中值滤波器头文件
├── median-filter.cpp          # 中值滤波器实现文件
└── lowpass-filter.hpp         # 原始低通滤波器（参考）

wibotlib/example/
├── median-filter-example.cpp  # 中值滤波器使用示例
└── lowpass-filter-example.cpp # 低通滤波器示例（参考）

test_median_filter.cpp         # 中值滤波器功能测试
```

## 主要特性

### 1. 多通道支持
- 支持编译时确定的多通道数量（模板参数`CHANNELS`）
- 每个通道维护独立的滤波状态和缓冲区
- 所有通道共享相同的滤波配置

### 2. 中值滤波算法
- 使用滑动窗口维护历史数据
- 采用快速选择算法计算中值，避免完整排序
- 支持奇数和偶数窗口大小
  - 奇数窗口：返回排序后中间位置的值
  - 偶数窗口：返回排序后中间两个值的平均值

### 3. 外部缓冲区管理
- **用户控制内存**: 缓冲区由用户在外部分配和管理
- **零内部分配**: 滤波器内部完全不进行动态内存分配
- **灵活的内存策略**: 支持栈分配、静态分配或堆分配
- **可配置大小**: 最大窗口大小由用户在配置中指定
- **内存复用**: 多个滤波器实例可以共享缓冲区池

### 4. 配置验证
- 窗口大小必须在1到`maxWindowSize`范围内
- 最大窗口大小由用户配置决定
- 提供静态配置验证函数确保参数有效性

## 使用方法

### 基本用法

```cpp
#include "wibotlib/src/SyncPipeline/filter/median-filter.hpp"
using namespace wibot;

// 1. 分配外部缓冲区
constexpr uint8_t MAX_WINDOW = 16;
float channelBuffer[MAX_WINDOW];
float tempBuffer[MAX_WINDOW];
float* bufferPtrs[1] = {channelBuffer};

// 2. 创建数据源
TestDataSource<1> dataSource;

// 3. 配置中值滤波器
MedianFilter<1>::Config config = {
    .windowSize = 5,        // 5点窗口
    .maxWindowSize = MAX_WINDOW  // 缓冲区最大大小
};

// 4. 创建滤波器（传入外部缓冲区）
MedianFilter<1> filter(dataSource, config, bufferPtrs, tempBuffer);

// 5. 使用滤波器
dataSource.setValue(0, inputValue);
filter.update();
float filteredValue = filter.getValue(0);
```

### 多通道使用

```cpp
// 1. 分配多通道缓冲区
constexpr uint8_t CHANNELS = 3;
constexpr uint8_t MAX_WINDOW = 16;
float channelBuffers[CHANNELS][MAX_WINDOW];
float tempBuffer[MAX_WINDOW];

// 2. 创建缓冲区指针数组
float* bufferPtrs[CHANNELS];
for (uint8_t i = 0; i < CHANNELS; i++) {
    bufferPtrs[i] = channelBuffers[i];
}

// 3. 创建3通道中值滤波器
TestDataSource<3> multiSource;
MedianFilter<3>::Config config = {
    .windowSize = 7,
    .maxWindowSize = MAX_WINDOW
};
MedianFilter<3> multiFilter(multiSource, config, bufferPtrs, tempBuffer);

// 4. 为每个通道设置不同的输入
multiSource.setValue(0, ch0_input);
multiSource.setValue(1, ch1_input);  
multiSource.setValue(2, ch2_input);

multiFilter.update();

// 5. 获取各通道的滤波结果
float ch0_output = multiFilter.getValue(0);
float ch1_output = multiFilter.getValue(1);
float ch2_output = multiFilter.getValue(2);
```

### 配置更新

```cpp
// 更新滤波器配置（注意：不能超过原始maxWindowSize）
MedianFilter<1>::Config newConfig = {
    .windowSize = 9,
    .maxWindowSize = MAX_WINDOW  // 保持与原配置一致
};
filter.updateConfig(newConfig);  // 重置缓冲区状态，无内存重分配

// 验证配置有效性
if (MedianFilter<1>::isConfigValid(newConfig)) {
    // 配置有效
}
```

## API参考

### 构造函数
```cpp
MedianFilter(SyncPipeline<float>& upstream, const Config& config);
```
- `upstream`: 上游数据管道
- `config`: 滤波器配置

### 主要方法
```cpp
float getValue(uint8_t channel) const;        // 获取指定通道的滤波结果
void update();                                // 更新滤波器状态
void reset();                                 // 重置所有通道状态
void updateConfig(const Config& config);      // 更新配置
static bool isConfigValid(const Config& config);  // 验证配置
```

### 配置结构
```cpp
struct Config {
    uint8_t windowSize;    // 当前滤波窗口大小
    uint8_t maxWindowSize; // 缓冲区最大窗口大小（用于验证）
};
```

### 构造函数
```cpp
MedianFilter(SyncPipeline<float>& upstream, const Config& config, 
             float* buffers[CHANNELS], float* tempBuffer);
```
- `upstream`: 上游数据管道
- `config`: 滤波器配置
- `buffers`: 外部缓冲区指针数组，每个指向大小为[maxWindowSize]的数组
- `tempBuffer`: 临时计算缓冲区，大小为[maxWindowSize]

## 与低通滤波器的对比

| 特性         | 中值滤波器     | 低通滤波器   |
| ------------ | -------------- | ------------ |
| 脉冲噪声抑制 | 优秀           | 一般         |
| 边缘保持     | 优秀           | 较差         |
| 平滑效果     | 保持突变       | 平滑突变     |
| 计算复杂度   | 较高           | 较低         |
| 内存需求     | 需要缓冲区     | 仅需状态变量 |
| 适用场景     | 脉冲噪声、毛刺 | 高频噪声     |

## 实现细节

### 1. 完全静态内存布局
采用完全静态分配策略，实现零动态内存分配：
```cpp
// 所有数据结构在编译时确定大小（完全静态分配）
float   _buffers[CHANNELS][MAX_WINDOW_SIZE];  // 各通道环形缓冲区（静态二维数组）
uint8_t _bufferIndex[CHANNELS];               // 各通道缓冲区当前索引
uint8_t _bufferCount[CHANNELS];               // 各通道缓冲区有效数据数量  
float   _outputLast[CHANNELS];                // 各通道上次输出值
float   _tempBuffer[MAX_WINDOW_SIZE];         // 临时缓冲区（静态数组）
```

**关键常量**:
```cpp
static constexpr uint8_t MAX_WINDOW_SIZE = 32;  // 支持的最大窗口大小
```

### 2. 快速选择算法
使用快速选择算法计算中值，时间复杂度O(n)：
```cpp
float _quickSelect(float* arr, int left, int right, int k);
int _partition(float* arr, int left, int right);
```

### 3. 内存安全
- 构造函数中动态分配内存
- 析构函数自动释放内存  
- 配置更新时重新分配内存

## 测试和示例

### 运行示例
```bash
# 编译并运行使用示例
g++ -std=c++17 -I. wibotlib/example/median-filter-example.cpp wibotlib/src/SyncPipeline/filter/median-filter.cpp -o median-example
./median-example

# 编译并运行功能测试
g++ -std=c++17 -I. test_median_filter.cpp wibotlib/src/SyncPipeline/filter/median-filter.cpp -o test-median
./test-median
```

### 测试用例
- 基本功能测试：验证中值计算的正确性
- 脉冲噪声抑制测试：验证对突发噪声的抑制效果
- 多通道功能测试：验证各通道独立工作
- 配置验证测试：验证配置参数的有效性检查
- 重置功能测试：验证状态重置的正确性

## 注意事项

1. **窗口大小选择**: 建议使用奇数窗口大小以获得更直观的中值结果
2. **内存使用**: 
   - 总内存大小: `CHANNELS * MAX_WINDOW_SIZE * sizeof(float) + CHANNELS * (2*sizeof(uint8_t) + sizeof(float)) + MAX_WINDOW_SIZE * sizeof(float)`
   - 全部栈上分配，无堆内存使用
   - 例如4通道: `4 * 32 * 4 + 4 * 6 + 32 * 4 = 664` 字节
3. **计算延迟**: 滤波器需要填满窗口后才能输出准确的中值
4. **线程安全**: 当前实现不是线程安全的，多线程使用时需要外部同步

## 优化亮点

本实现采用完全静态内存分配，相比传统动态分配设计具有以下优势：

1. **零动态分配**: 完全避免`new`/`delete`，所有内存在编译时确定
2. **实时系统友好**: 无内存分配延迟，适合实时控制系统
3. **内存安全**: 从根本上避免内存泄漏、双重释放等问题
4. **栈上分配**: 所有数据在栈上，提供最佳的访问性能
5. **编译时优化**: 编译器能够充分优化静态数组访问
6. **可预测性**: 内存使用量在编译时就已确定

### 性能对比

| 特性     | 传统设计（动态分配） | 当前设计（静态分配） |
| -------- | -------------------- | -------------------- |
| 内存分配 | 运行时`new`/`delete` | 编译时栈分配         |
| 访问性能 | 指针间接访问         | 直接数组访问         |
| 内存布局 | 分散在堆上           | 连续的栈空间         |
| 缓存效率 | 较差                 | 优秀                 |
| 分配开销 | 存在运行时开销       | 零开销               |
| 内存安全 | 需要手动管理         | 自动安全             |
| 实时性   | 不确定的分配延迟     | 完全确定性           |

## 扩展建议

1. **加权中值滤波**: 为窗口中的不同位置分配不同权重
2. **自适应窗口**: 根据信号特性动态调整窗口大小
3. **混合滤波**: 结合中值滤波和低通滤波的优点
4. **优化算法**: 使用增量更新算法减少计算量
5. **SIMD优化**: 利用向量指令加速中值计算
# CircularBuffer 循环缓冲区使用指南

## 概述

`CircularBuffer` 是 WibotLib 提供的高性能循环缓冲区（环形缓冲区）实现，采用镜像标志位技术，特别适用于嵌入式系统中的数据流处理、DMA传输、串口通信等场景。

**文件位置**：[libs/wibotlib/src/base/circular-buffer.hpp](../src/base/circular-buffer.hpp)

## 核心特性

### 1. 镜像标志位技术

使用镜像标志位（Mirror Flag）技术来区分缓冲区的空和满状态，相比传统方法具有以下优势：

- **无浪费空间**：所有缓冲区空间都可用，不需要预留一个位置来区分空/满
- **高效判断**：通过简单的位运算即可判断空/满状态
- **支持覆盖写入**：适合高速数据流处理

**原理**：
- 容量必须是2的幂（如：16、32、64、128、256）
- 使用逻辑索引空间是物理空间的2倍
- 通过读写索引的镜像位区分一圈的差异

```
物理空间: [0, 1, 2, 3, 4, 5, 6, 7]  (capacity = 8)
逻辑空间: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]

当 write=8, read=0 时，缓冲区满（写索引领先一圈）
当 write=0, read=0 时，缓冲区空（读写索引相同）
```

### 2. 零拷贝支持

提供指针访问方法，支持 DMA 等硬件直接访问缓冲区，避免数据拷贝：
- `getWritePtr()`: 获取写入位置指针
- `getReadPtr()`: 获取读取位置指针
- `writeVirtual()`: 虚拟写入（仅移动指针）
- `readVirtual()`: 虚拟读取（仅移动指针）

### 3. 灵活的覆盖策略

支持两种写入模式：
- **覆盖模式**：缓冲区满时自动覆盖旧数据（适合实时数据流）
- **非覆盖模式**：缓冲区满时拒绝写入（适合不可丢失的数据）

## 类定义

```cpp
template <typename TE>
class CircularBuffer {
public:
    CircularBuffer(Slice buffer);
    
    // 容量查询
    u32 getCapacity() const;        // 逻辑容量（元素数量）
    u32 getMemCapacity() const;     // 内存容量（字节数）
    u32 getDataWidth() const;       // 元素宽度（字节）
    
    // 状态查询
    bool isFull() const;            // 是否已满
    bool isEmpty() const;           // 是否为空
    u32 getSize() const;            // 当前元素数量
    u32 getSpace() const;           // 剩余空间
    
    // 数据读写
    u32 write(const TE *data, u32 length, bool allowCover = true);
    u32 read(TE *data, u32 length);
    u32 peek(TE *data, u32 start = 0, u32 length = 1);
    
    // 虚拟读写（DMA支持）
    bool writeVirtual(u32 length);
    bool readVirtual(u32 length);
    
    // 指针访问（零拷贝）
    TE *getWritePtr() const;
    TE *getReadPtr() const;
    TE *peekPtr(u32 offset, bool force = false);
    TE *getDataPtr();
    
    // 高级功能
    u32 getSizeWithoutMemWrap() const;
    u32 getLengthByMemIndex(u32 end, u32 start);
    Result clear();
};

// 类型别名
typedef CircularBuffer<u8> CircularBuffer8;  // 字节缓冲区
```

## 基本使用

### 1. 创建循环缓冲区

```cpp
#include "circular-buffer.hpp"
#include "buffer.hpp"

using namespace wibot;

// 方法1：使用 Buffer 创建（推荐）
Buffer<256> storage;  // 必须是2的幂
CircularBuffer8 cb(storage);

// 方法2：使用原始数组
u8 rawBuffer[128];
Slice slice(rawBuffer, 128);
CircularBuffer8 cb2(slice);

// 方法3：指定元素类型
Buffer<64, u16> u16Storage;
CircularBuffer<u16> cb16(u16Storage);
```

**重要**：缓冲区大小必须是2的幂（2, 4, 8, 16, 32, 64, 128, 256...）

### 2. 写入数据

```cpp
u8 data[] = {0x01, 0x02, 0x03, 0x04, 0x05};

// 覆盖模式（默认）：缓冲区满时覆盖旧数据
u32 written = cb.write(data, 5, true);
// written = 5

// 非覆盖模式：缓冲区满时只写入能写的部分
u32 written2 = cb.write(data, 5, false);
// 如果空间不足，written2 < 5
```

### 3. 读取数据

```cpp
u8 buffer[10];

// 读取数据（从缓冲区移除）
u32 read = cb.read(buffer, 10);
// 实际读取的字节数可能小于请求数量

// 检查读取结果
if (read > 0) {
    // 处理读取的数据
    for (u32 i = 0; i < read; i++) {
        printf("0x%02X ", buffer[i]);
    }
}
```

### 4. 查看数据（不移除）

```cpp
u8 buffer[5];

// 查看前5个字节（不从缓冲区移除）
u32 peeked = cb.peek(buffer, 0, 5);

// 查看偏移位置的数据
u32 peeked2 = cb.peek(buffer, 10, 5);  // 从第10个字节开始查看5个
```

### 5. 状态检查

```cpp
// 获取容量和大小
u32 capacity = cb.getCapacity();  // 缓冲区总容量
u32 size = cb.getSize();          // 当前数据量
u32 space = cb.getSpace();        // 剩余空间

// 状态判断
if (cb.isEmpty()) {
    // 缓冲区为空
}

if (cb.isFull()) {
    // 缓冲区已满
}

// 检查是否有足够空间
if (cb.getSpace() >= 10) {
    // 可以写入至少10个元素
}
```

### 6. 清空缓冲区

```cpp
Result result = cb.clear();
if (result.isOk()) {
    // 清空成功
}
```

## 高级功能

### 1. DMA 零拷贝操作

循环缓冲区支持 DMA 直接访问，避免数据拷贝：

```cpp
// DMA 写入示例
void dmaWriteCallback() {
    // 获取写入指针
    u8 *writePtr = cb.getWritePtr();
    u32 space = cb.getSpace();
    
    // 启动 DMA 传输到缓冲区
    HAL_UART_Receive_DMA(&huart1, writePtr, space);
}

void dmaTxCompleteCallback(u32 length) {
    // DMA 传输完成，虚拟写入（仅移动指针）
    bool overflow = cb.writeVirtual(length);
    
    if (overflow) {
        LOG_W("Buffer overflow occurred");
    }
}

// DMA 读取示例
void dmaReadCallback() {
    // 获取读取指针
    u8 *readPtr = cb.getReadPtr();
    u32 size = cb.getSizeWithoutMemWrap();  // 连续可读数据
    
    // 启动 DMA 传输从缓冲区
    HAL_UART_Transmit_DMA(&huart1, readPtr, size);
}

void dmaRxCompleteCallback(u32 length) {
    // DMA 传输完成，虚拟读取（仅移动指针）
    bool overflow = cb.readVirtual(length);
    
    if (overflow) {
        LOG_W("Read beyond available data");
    }
}
```

**关键方法**：
- `getWritePtr()`: 获取当前写入位置的指针
- `getReadPtr()`: 获取当前读取位置的指针
- `writeVirtual(length)`: 告知已写入 length 个元素（仅移动指针）
- `readVirtual(length)`: 告知已读取 length 个元素（仅移动指针）
- `getSizeWithoutMemWrap()`: 获取不跨越边界的连续数据长度

### 2. 指针访问和窥视

```cpp
// 获取指定偏移位置的指针（只读）
u8 *ptr = cb.peekPtr(5);  // 获取第5个元素的指针
if (ptr != nullptr) {
    u8 value = *ptr;
}

// 强制获取指针（即使超出范围）
u8 *forcePtr = cb.peekPtr(100, true);

// 获取底层缓冲区指针
u8 *rawPtr = cb.getDataPtr();
```

### 3. 内存索引计算

```cpp
// 获取连续可读数据长度（不跨越内存边界）
u32 contiguous = cb.getSizeWithoutMemWrap();

// 计算两个内存索引之间的距离
u32 length = cb.getLengthByMemIndex(endIndex, startIndex);
```

## 应用场景和示例

### 示例 1：串口接收缓冲

```cpp
#include "circular-buffer.hpp"
#include "uart.hpp"

class UartRxBuffer {
public:
    UartRxBuffer() : buffer(storage) {}
    
    // 初始化
    void init() {
        // 启动 UART DMA 接收
        u8 *ptr = buffer.getWritePtr();
        u32 space = buffer.getSpace();
        HAL_UART_Receive_DMA(&huart1, ptr, space);
    }
    
    // DMA 半完成回调
    void onDmaHalfComplete(u32 length) {
        buffer.writeVirtual(length);
        processData();
    }
    
    // DMA 完成回调
    void onDmaComplete(u32 length) {
        buffer.writeVirtual(length);
        processData();
        
        // 重启 DMA
        init();
    }
    
    // 读取接收到的数据
    u32 read(u8 *data, u32 maxLen) {
        return buffer.read(data, maxLen);
    }
    
    // 查找特定字符
    bool findChar(u8 ch, u32 &position) {
        u32 size = buffer.getSize();
        for (u32 i = 0; i < size; i++) {
            u8 *ptr = buffer.peekPtr(i);
            if (ptr && *ptr == ch) {
                position = i;
                return true;
            }
        }
        return false;
    }
    
private:
    void processData() {
        // 处理接收到的数据
        u8 byte;
        while (buffer.read(&byte, 1) > 0) {
            // 处理每个字节
            processRxByte(byte);
        }
    }
    
    void processRxByte(u8 byte) {
        // 数据处理逻辑
    }
    
    Buffer<256> storage;
    CircularBuffer8 buffer;
};
```

### 示例 2：传感器数据采样

```cpp
#include "circular-buffer.hpp"

class SensorDataBuffer {
public:
    struct SensorData {
        u32 timestamp;
        f32 temperature;
        f32 humidity;
        f32 pressure;
    };
    
    SensorDataBuffer() : buffer(storage) {}
    
    // 添加采样数据
    void addSample(const SensorData &data) {
        // 覆盖模式：保持最新的数据
        buffer.write(&data, 1, true);
    }
    
    // 获取最近的 N 个样本
    u32 getRecentSamples(SensorData *samples, u32 count) {
        return buffer.read(samples, count);
    }
    
    // 计算平均值（不移除数据）
    SensorData calculateAverage() {
        u32 size = buffer.getSize();
        if (size == 0) {
            return {0, 0.0f, 0.0f, 0.0f};
        }
        
        SensorData sum = {0, 0.0f, 0.0f, 0.0f};
        
        for (u32 i = 0; i < size; i++) {
            SensorData *ptr = buffer.peekPtr(i);
            if (ptr) {
                sum.temperature += ptr->temperature;
                sum.humidity += ptr->humidity;
                sum.pressure += ptr->pressure;
            }
        }
        
        return {
            0,
            sum.temperature / size,
            sum.humidity / size,
            sum.pressure / size
        };
    }
    
    // 获取数据量
    u32 getSampleCount() const {
        return buffer.getSize();
    }
    
    // 清空历史数据
    void clear() {
        buffer.clear();
    }
    
private:
    Buffer<64, SensorData> storage;  // 存储64个样本
    CircularBuffer<SensorData> buffer;
};

// 使用示例
void sensorTask() {
    SensorDataBuffer dataBuffer;
    
    // 模拟采样
    for (int i = 0; i < 100; i++) {
        SensorDataBuffer::SensorData sample = {
            System::getTick(),
            25.0f + (i % 10) * 0.1f,  // 温度
            60.0f + (i % 10) * 0.5f,  // 湿度
            1013.0f + (i % 10) * 0.2f // 压力
        };
        
        dataBuffer.addSample(sample);
        
        if (dataBuffer.getSampleCount() >= 10) {
            // 计算最近10个样本的平均值
            auto avg = dataBuffer.calculateAverage();
            LOG_I("Avg Temp: %.2f, Humidity: %.2f, Pressure: %.2f",
                  avg.temperature, avg.humidity, avg.pressure);
        }
        
        os::sleep(100);
    }
}
```

### 示例 3：音频数据缓冲

```cpp
#include "circular-buffer.hpp"

class AudioBuffer {
public:
    AudioBuffer() : buffer(storage) {}
    
    // 生产者：ADC采样数据写入
    void onAdcSample(i16 sample) {
        buffer.write(&sample, 1, true);  // 覆盖模式
    }
    
    // 消费者：音频处理
    void processAudio() {
        constexpr u32 FRAME_SIZE = 128;
        i16 frame[FRAME_SIZE];
        
        // 只在有完整帧时处理
        if (buffer.getSize() >= FRAME_SIZE) {
            u32 read = buffer.read(frame, FRAME_SIZE);
            
            if (read == FRAME_SIZE) {
                // 音频处理（FFT、滤波等）
                processAudioFrame(frame, FRAME_SIZE);
            }
        }
    }
    
    // DMA 批量采样
    void startDmaAdc() {
        i16 *writePtr = buffer.getWritePtr();
        u32 space = buffer.getSpace();
        
        // 启动 ADC DMA
        HAL_ADC_Start_DMA(&hadc1, (u32*)writePtr, space);
    }
    
    void onDmaComplete(u32 samples) {
        buffer.writeVirtual(samples);
        processAudio();
    }
    
private:
    void processAudioFrame(i16 *samples, u32 count) {
        // FFT、滤波等音频处理
    }
    
    Buffer<2048, i16> storage;  // 2048个16位采样
    CircularBuffer<i16> buffer;
};
```

### 示例 4：日志缓冲

```cpp
#include "circular-buffer.hpp"

class LogBuffer {
public:
    LogBuffer() : buffer(storage) {}
    
    // 添加日志消息
    void log(const char *message) {
        u32 len = strlen(message);
        
        // 非覆盖模式：避免截断日志
        u32 written = buffer.write((u8*)message, len, false);
        
        if (written < len) {
            LOG_W("Log buffer full, %d bytes discarded", len - written);
        }
        
        // 添加换行符
        const u8 newline = '\n';
        buffer.write(&newline, 1, false);
    }
    
    // 发送日志到串口
    void flush(Uart &uart) {
        u8 chunk[64];
        
        while (buffer.getSize() > 0) {
            u32 read = buffer.read(chunk, sizeof(chunk));
            uart.write(chunk, read);
        }
    }
    
    // 查找日志行
    bool findLine(char *line, u32 maxLen) {
        u32 size = buffer.getSize();
        
        for (u32 i = 0; i < size; i++) {
            u8 *ptr = buffer.peekPtr(i);
            if (ptr && *ptr == '\n') {
                // 找到换行符
                u32 lineLen = i + 1;
                if (lineLen <= maxLen) {
                    buffer.read((u8*)line, lineLen);
                    line[lineLen - 1] = '\0';  // 替换换行为结束符
                    return true;
                }
            }
        }
        
        return false;
    }
    
private:
    Buffer<1024> storage;
    CircularBuffer8 buffer;
};

// 使用示例
LogBuffer logBuffer;

void logTask() {
    logBuffer.log("System started");
    logBuffer.log("Loading configuration...");
    logBuffer.log("Initializing peripherals...");
    
    // 定期刷新到串口
    logBuffer.flush(uart);
}
```

## 性能考虑

### 1. 内存要求

- **容量必须是2的幂**：这是镜像标志位技术的要求
- **无额外开销**：除了底层数组外，只需 16 字节存储指针和索引
- **静态分配**：推荐使用 `Buffer` 进行栈上或静态分配

### 2. 时间复杂度

- **写入/读取**：O(n)，其中 n 是数据长度
- **状态查询**：O(1)，常数时间
- **指针获取**：O(1)，无内存拷贝

### 3. 线程安全

`CircularBuffer` **不是线程安全的**，在多线程环境下需要外部同步：

```cpp
// 使用互斥锁保护
Mutex mutex;

void writerThread() {
    mutex.lock();
    cb.write(data, size);
    mutex.unlock();
}

void readerThread() {
    mutex.lock();
    cb.read(buffer, size);
    mutex.unlock();
}
```

**注意**：
- DMA 写入时避免 CPU 同时写入
- 中断中使用时禁用相关中断或使用临界区

## 常见问题

### Q1: 为什么容量必须是2的幂？

镜像标志位技术使用位运算来高效地进行索引映射：
```cpp
#define WrapMemIndex(a)   ((a) & (_capacity - 1))
```
只有当容量是2的幂时，`_capacity - 1` 的所有低位才是1，可以作为掩码使用。

### Q2: 覆盖模式下旧数据何时被丢弃？

在写入新数据时，如果缓冲区空间不足，旧数据会被自动覆盖。读指针会自动前移，确保读到的总是有效数据。

### Q3: 如何判断 DMA 传输是否溢出？

`writeVirtual()` 和 `readVirtual()` 会返回 `bool` 值：
```cpp
bool overflow = cb.writeVirtual(length);
if (overflow) {
    // 发生溢出，数据可能丢失
}
```

### Q4: 可以存储复杂对象吗？

可以，但需要注意：
```cpp
struct ComplexObject {
    u32 id;
    f32 data[10];
    // 不要包含指针或动态分配的成员
};

CircularBuffer<ComplexObject> cb(buffer);
```

**注意**：避免存储包含指针、虚函数或需要深拷贝的对象。

### Q5: 如何选择合适的缓冲区大小？

考虑以下因素：
- **数据速率**：缓冲区应能容纳足够的数据以应对处理延迟
- **突发性**：考虑数据的突发特性
- **内存限制**：在嵌入式系统中平衡内存使用

```cpp
// 示例：串口接收
// 波特率 115200, 字节率约 11520 字节/秒
// 处理周期 10ms，预留 2 倍余量
u32 bufferSize = (11520 * 0.01 * 2);  // ≈ 230，向上取2的幂 = 256
Buffer<256> storage;
```

## 最佳实践

1. **使用编译时常量定义大小**
   ```cpp
   constexpr u32 RX_BUFFER_SIZE = 256;
   Buffer<RX_BUFFER_SIZE> storage;
   ```

2. **检查返回值**
   ```cpp
   u32 written = cb.write(data, size);
   if (written < size) {
       LOG_W("Only %d of %d bytes written", written, size);
   }
   ```

3. **DMA 使用连续空间**
   ```cpp
   u32 contiguous = cb.getSizeWithoutMemWrap();
   HAL_UART_Transmit_DMA(&huart1, cb.getReadPtr(), contiguous);
   ```

4. **避免频繁清空**
   ```cpp
   // ❌ 低效
   while (!cb.isEmpty()) {
       cb.read(&byte, 1);
   }
   
   // ✅ 高效
   cb.clear();
   ```

5. **合理选择覆盖模式**
   - 实时数据流：使用覆盖模式（`allowCover=true`）
   - 关键数据：使用非覆盖模式（`allowCover=false`）并检查返回值

## 相关文档

- [基础组件使用指南](BASE_FUNDAMENTALS.md)
- [异步编程指南](async-result-usage.md)
- [UART 使用指南](UART_USAGE.md)

## 总结

`CircularBuffer` 是一个高效、灵活的循环缓冲区实现：

✅ **零浪费空间**：镜像标志位技术充分利用缓冲区  
✅ **DMA 友好**：支持零拷贝操作  
✅ **灵活覆盖策略**：适应不同应用需求  
✅ **类型安全**：模板支持任意元素类型  
✅ **高性能**：O(1) 状态查询，O(n) 数据传输  

特别适合串口通信、音频处理、传感器采样等高速数据流场景。

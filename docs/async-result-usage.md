# AsyncResult 和 AsyncSource 异步编程模型

这是一套基于 FreeRTOS EventGroup 实现的异步编程机制，允许在嵌入式系统中优雅地处理异步操作。

## 设计理念

- **AsyncSource**: 异步操作的数据源，负责在操作完成时发出信号
- **AsyncResult**: 异步操作的代理，提供等待和获取结果的接口
- **EventGroupPool**: 管理有限的 EventGroup 资源，避免系统资源浪费

## 核心特性

1. **资源管理**: 使用对象池管理 EventGroup，避免频繁创建销毁
2. **移动语义**: 支持 C++11 移动语义，避免不必要的拷贝
3. **超时控制**: 支持带超时的等待机制
4. **错误处理**: 统一的错误处理机制

## 使用方法

### 基本用法

```cpp
// 1. 创建异步源
AsyncSource source = AsyncSource::create();

// 2. 获取异步结果
AsyncResult result = source.getResult();

// 3. 启动异步操作（在另一个线程或中断中）
// ... 异步操作进行中 ...

// 4. 在操作完成时设置结果
source.setDone();  // 成功完成
// 或
source.setError(Result::kError);  // 操作失败

// 5. 等待结果
Result waitResult = result.wait(1000);  // 等待最多1000ms
if (waitResult.isOk()) {
    // 操作成功完成
} else if (waitResult.isTimeout()) {
    // 操作超时
} else {
    // 操作失败
}
```

### 实际应用示例

```cpp
class StreamReader {
public:
    StreamReader() : _source(AsyncSource::create()) {}
    
    AsyncResult read(Slice& data) {
        // 启动DMA或其他异步读取操作
        startAsyncRead(data);
        return _source.getResult();
    }
    
    void _isr_complete() {
        // 在中断服务例程中调用
        _source.setDone();
    }
    
    void _isr_error() {
        // 在错误中断中调用
        _source.setError(Result::kError);
    }
    
private:
    AsyncSource _source;
    
    void startAsyncRead(Slice& data) {
        // 启动硬件异步读取
        // 设置DMA、中断等
    }
};

class StreamWorker {
public:
    void run() {
        while (true) {
            DataFrame frame;
            AsyncResult ar = _stream.read(frame);
            
            Result rst = ar.wait(1000);
            if (rst.isOk()) {
                // 处理接收到的数据
                processFrame(frame);
            } else if (rst.isTimeout()) {
                // 处理超时
                handleTimeout();
            } else {
                // 处理错误
                handleError(rst);
            }
        }
    }
    
private:
    StreamReader _stream;
};
```

## API 参考

### AsyncSource

#### 静态方法
- `static AsyncSource create()`: 创建一个新的异步源

#### 实例方法
- `void setDone()`: 设置异步操作成功完成
- `void setError(Result result)`: 设置异步操作失败
- `AsyncResult getResult()`: 获取关联的异步结果对象

### AsyncResult

#### 实例方法
- `Result wait(uint32_t timeout = TIMEOUT_FOREVER)`: 等待异步操作完成
  - `timeout`: 超时时间（毫秒），默认为永不超时
  - 返回值: `Result::kOk` 表示成功，`Result::kTimeout` 表示超时，其他值表示错误

#### 移动语义
- 支持移动构造和移动赋值
- 禁用拷贝构造和拷贝赋值

## 配置参数

```cpp
#define EVENT_POOL_SIZE 2        // EventGroup 池大小
#define TIMEOUT_NOWAIT  0x00000000
#define TIMEOUT_FOREVER 0xFFFFFFFF
```

## 注意事项

1. **资源限制**: EventGroup 池大小有限（默认为2），同时进行的异步操作数量受限
2. **线程安全**: 所有操作都是线程安全的，可以在中断和任务中安全使用
3. **生命周期**: AsyncSource 必须在 AsyncResult 之前销毁
4. **重用**: 同一个 AsyncSource 可以多次使用，但需要在上次操作完成后才能开始新操作

## 错误处理

可能的错误类型：
- `Result::kOk`: 操作成功
- `Result::kTimeout`: 等待超时
- `Result::kError`: 一般错误
- `Result::kNoResource`: 没有可用的 EventGroup 资源

## 性能考虑

1. EventGroup 池避免了频繁的系统资源分配
2. 移动语义减少了对象拷贝的开销
3. 原子操作确保线程安全性，同时保持高性能
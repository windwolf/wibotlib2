# wibotlib OS 使用说明

## 概述

wibotlib的OS模块提供了跨平台的实时操作系统(RTOS)抽象层，支持多种RTOS后端：
- **ThreadX** (Azure RTOS)
- **FreeRTOS**
- **No-OS** (裸机环境)

通过统一的API接口，应用代码可以在不同RTOS之间无缝切换，无需修改业务逻辑代码。

## RTOS选择与配置

### 编译时选择

在项目的CMakeLists.txt或编译选项中定义以下宏之一：

```cmake
# ThreadX
add_definitions(-DTHREADX)

# FreeRTOS
add_definitions(-DFREERTOS)

# 裸机环境
add_definitions(-DNORTOS)
```

### 头文件包含

```cpp
#include "os.hpp"  // 包含所有OS接口
```

## 核心组件

### 1. 上下文检测

#### 获取当前运行上下文

```cpp
#include "os.hpp"

namespace wibot::os {
enum class ContextMode {
    kThread = 0,  // 线程上下文
    kISR    = 1,  // 中断服务例程
    kInit   = 2,  // 初始化阶段（RTOS未启动）
};

ContextMode getContextMode();  // 获取当前上下文
bool isInThread();             // 是否在线程中运行
}
```

**使用示例**：
```cpp
void myFunction() {
    if (os::isInThread()) {
        // 可以安全地调用阻塞操作
        os::sleep(100);
    } else if (os::getContextMode() == os::ContextMode::kISR) {
        // 在中断中，只能使用非阻塞操作
        LOG_W("Cannot block in ISR");
    }
}
```

### 2. 时间管理

#### 延时函数

```cpp
namespace wibot::os {
uint32_t getTickMs();      // 获取系统滴答计数（毫秒）
void sleep(uint32_t ms);   // 延时指定毫秒数
}
```

**使用示例**：
```cpp
void periodicTask() {
    while (true) {
        doWork();
        os::sleep(1000);  // 延时1秒
    }
}
```

**注意事项**：
- `sleep()`只能在线程上下文中调用
- 在ISR或初始化阶段调用会触发断言错误
- 裸机模式下会回退到忙等待延时

### 3. 线程 (Thread)

#### 线程创建

```cpp
class Worker {
public:
    virtual void run() = 0;  // 线程入口函数
};

template <u16 stack_size>
class Thread {
public:
    Thread(const char* name,       // 线程名称
           Worker* worker,         // 工作对象
           uint32_t priority,      // 优先级
           const ThreadConfig& config = ThreadConfig());
    ~Thread();
    
    void start();  // 启动线程
};
```

**使用示例**：
```cpp
class MyTask : public Worker {
public:
    void run() override {
        while (true) {
            // 执行任务
            LOG_I("Task running");
            os::sleep(500);
        }
    }
};

// 创建线程
MyTask myTask;
Thread<2048> myThread("MyThread", &myTask, 10);  // 2KB栈，优先级10

// 在合适的时机启动
myThread.start();
```

**参数说明**：
- `stack_size`：栈大小（字节）
- `priority`：优先级数值（具体范围取决于RTOS）
  - ThreadX: 0-31，数值越小优先级越高
  - FreeRTOS: 0-configMAX_PRIORITIES-1，数值越大优先级越高
- `ThreadConfig`：额外配置（如抢占阈值、时间片等）

### 4. 定时器 (OsTimer)

#### 定时器创建

```cpp
class OsTimer {
public:
    OsTimer(const char* name,     // 定时器名称
            Worker* worker,       // 回调工作对象
            uint32_t period,      // 周期（毫秒）
            uint32_t firstDelay); // 首次触发延时
    ~OsTimer();
    
    void start();  // 启动定时器
    void stop();   // 停止定时器
};
```

**使用示例**：
```cpp
class TimerTask : public Worker {
public:
    void run() override {
        LOG_I("Timer triggered");
        // 执行定时任务
    }
};

TimerTask timerTask;
OsTimer timer("MyTimer", &timerTask, 1000, 500);  // 500ms后首次触发，之后每1s触发

timer.start();  // 启动定时器

// 需要时停止
timer.stop();
```

### 5. 互斥锁 (Mutex)

#### 互斥锁创建与使用

```cpp
class Mutex {
public:
    explicit Mutex(const char* name);
    ~Mutex();
    
    Result lock(uint32_t timeout);   // 获取锁
    void unlock();                    // 释放锁
};
```

**使用示例**：
```cpp
Mutex dataMutex("DataMutex");
int sharedData = 0;

void updateData(int value) {
    Result rst = dataMutex.lock(TIMEOUT_FOREVER);  // 永久等待
    if (rst.isOk()) {
        sharedData = value;  // 临界区
        dataMutex.unlock();
    }
}

void readData() {
    if (dataMutex.lock(100).isOk()) {  // 最多等待100ms
        int value = sharedData;
        dataMutex.unlock();
        LOG_I("Data: %d", value);
    } else {
        LOG_W("Failed to acquire mutex");
    }
}
```

**超时常量**：
```cpp
#define TIMEOUT_FOREVER  0xFFFFFFFF  // 永久等待
#define TIMEOUT_NO_WAIT  0           // 立即返回
```

### 6. 事件组 (EventGroup)

事件组用于线程间的事件同步，支持多个事件标志位的逻辑组合。

#### 事件组创建与操作

```cpp
using EventFlag = uint32_t;

class EventGroup {
public:
    EventGroup();
    explicit EventGroup(const char* name);
    ~EventGroup();
    
    Result set(uint32_t flags);     // 设置标志位
    Result reset(uint32_t flags);   // 清除标志位
    Result wait(uint32_t flags,     // 等待标志位
                uint32_t& actual_flags,
                EventOptions options,
                uint32_t timeout);
    
    // 动态分配标志位
    EventFlag fetchFlag();          // 获取一个空闲标志位
    EventFlag fetchFlagPair();      // 获取两个相邻标志位
    void releaseFlag(EventFlag flag); // 释放标志位
};
```

#### 事件选项

```cpp
using EventOptions = uint8_t;

// 等待模式
constexpr EventOptions EventOptions_WaitForAny = 0x00;  // 任意标志满足
constexpr EventOptions EventOptions_WaitForAll = 0x02;  // 所有标志满足

// 清除模式
constexpr EventOptions EventOptions_NoClear = 0x00;     // 不自动清除
constexpr EventOptions EventOptions_Clear   = 0x01;     // 自动清除
```

**使用示例**：

```cpp
EventGroup events("MyEvents");

#define EVENT_TASK1_DONE  (1 << 0)
#define EVENT_TASK2_DONE  (1 << 1)
#define EVENT_ERROR       (1 << 2)

// 发送端
void task1() {
    // 完成工作
    events.set(EVENT_TASK1_DONE);
}

void task2() {
    // 完成工作
    events.set(EVENT_TASK2_DONE);
}

// 接收端
void coordinator() {
    uint32_t actualFlags;
    
    // 等待任一任务完成
    Result rst = events.wait(
        EVENT_TASK1_DONE | EVENT_TASK2_DONE,
        actualFlags,
        EventOptions_WaitForAny | EventOptions_Clear,
        5000  // 5秒超时
    );
    
    if (rst.isOk()) {
        if (actualFlags & EVENT_TASK1_DONE) {
            LOG_I("Task1 completed");
        }
        if (actualFlags & EVENT_TASK2_DONE) {
            LOG_I("Task2 completed");
        }
    } else {
        LOG_W("Timeout waiting for tasks");
    }
}
```

#### 动态标志位分配

当需要动态分配事件标志时（例如在异步操作中），可以使用标志位池：

```cpp
EventGroup sharedEvents;

// 分配标志位
EventFlag myFlag = sharedEvents.fetchFlag();
if (myFlag != 0) {
    // 使用标志位
    sharedEvents.set(myFlag);
    
    // 使用完毕后释放
    sharedEvents.releaseFlag(myFlag);
}

// 分配一对标志位（用于成功/失败两种状态）
EventFlag flagPair = sharedEvents.fetchFlagPair();
EventFlag successFlag = flagPair & (~(flagPair >> 1));
EventFlag errorFlag = flagPair & (~(flagPair << 1));
```

### 7. 消息队列 (MessageQueue)

#### 消息队列创建与使用

```cpp
class MessageQueue {
public:
    MessageQueue(const char* name,    // 队列名称
                 void* msgAddr,       // 消息缓冲区地址
                 uint32_t msgSize,    // 消息大小（字）
                 uint32_t queueSize); // 队列长度（消息数）
    ~MessageQueue();
    
    Result send(const void* msg, uint32_t timeout);      // 发送消息
    Result receive(void* msg, uint32_t timeout);         // 接收消息
    Result flush();                                      // 清空队列
};
```

**使用示例**：

```cpp
struct Message {
    uint32_t id;
    int32_t value;
};

// 分配消息缓冲区
constexpr size_t MSG_SIZE = sizeof(Message) / sizeof(uint32_t);
constexpr size_t QUEUE_SIZE = 10;
uint32_t msgBuffer[MSG_SIZE * QUEUE_SIZE];

MessageQueue queue("MsgQueue", msgBuffer, MSG_SIZE, QUEUE_SIZE);

// 生产者
void producer() {
    Message msg = {.id = 1, .value = 100};
    Result rst = queue.send(&msg, 1000);  // 1秒超时
    if (rst.isOk()) {
        LOG_I("Message sent");
    }
}

// 消费者
void consumer() {
    Message msg;
    Result rst = queue.receive(&msg, TIMEOUT_FOREVER);
    if (rst.isOk()) {
        LOG_I("Received: id=%u, value=%d", msg.id, msg.value);
    }
}
```

## 最佳实践

### 1. 资源管理

```cpp
// 使用RAII模式管理互斥锁
class MutexGuard {
public:
    explicit MutexGuard(Mutex& mutex) : mutex_(mutex) {
        mutex_.lock(TIMEOUT_FOREVER);
    }
    ~MutexGuard() {
        mutex_.unlock();
    }
private:
    Mutex& mutex_;
};

// 使用
void criticalFunction() {
    MutexGuard guard(dataMutex);
    // 自动加锁，退出作用域时自动解锁
    sharedData++;
}
```

### 2. 超时处理

```cpp
void robustOperation() {
    Result rst = mutex.lock(100);  // 使用合理的超时
    if (rst == Result::kTimeout) {
        LOG_W("Operation timeout");
        return;
    }
    
    // 执行操作
    mutex.unlock();
}
```

### 3. 优先级配置

```cpp
// 为不同类型的任务设置合适的优先级
constexpr uint32_t PRIORITY_HIGH   = 5;   // 高优先级（紧急任务）
constexpr uint32_t PRIORITY_NORMAL = 10;  // 普通优先级
constexpr uint32_t PRIORITY_LOW    = 15;  // 低优先级（后台任务）

Thread<2048> urgentTask("Urgent", &urgent, PRIORITY_HIGH);
Thread<2048> normalTask("Normal", &normal, PRIORITY_NORMAL);
Thread<2048> backgroundTask("Background", &bg, PRIORITY_LOW);
```

### 4. 栈大小选择

```cpp
// 根据任务复杂度选择合适的栈大小
Thread<512>  simpleTask("Simple", &simple, 10);      // 简单任务
Thread<2048> normalTask("Normal", &normal, 10);      // 常规任务
Thread<4096> complexTask("Complex", &complex, 10);   // 复杂任务（递归、大局部变量）
```

## 平台差异

### ThreadX vs FreeRTOS

| 特性         | ThreadX      | FreeRTOS    | No-OS  |
| ------------ | ------------ | ----------- | ------ |
| 优先级范围   | 0-31 (小=高) | 0-N (大=高) | 不支持 |
| 事件组标志位 | 32位         | 24位        | 32位   |
| 时间片调度   | 支持         | 支持        | 不支持 |
| 优先级继承   | 支持         | 支持        | 不支持 |

### 裸机模式限制

在No-OS模式下：
- 线程和定时器无法工作
- `sleep()`使用忙等待
- 互斥锁退化为简单的布尔标志
- 事件组使用原子操作实现

## 错误处理

所有阻塞操作都返回`Result`类型：

```cpp
Result rst = mutex.lock(100);

if (rst.isOk()) {
    // 成功
} else if (rst == Result::kTimeout) {
    // 超时
} else if (rst == Result::kError) {
    // 其他错误
}
```

## 调试技巧

### 1. 启用日志

```cpp
#include "logger.hpp"
LOGGER("my-module")

void myFunction() {
    LOG_I("Thread started");
    LOG_W("Low memory warning");
    LOG_E("Fatal error");
}
```

### 2. 断言检查

```cpp
ASSERT(os::isInThread(), "Must be called from thread context");
ASSERT(mutex.lock(0).isOk(), "Mutex already locked");
```

### 3. 死锁检测

- 使用超时而非永久等待
- 保持一致的锁获取顺序
- 使用RAII管理锁生命周期

## 示例：完整的多线程应用

```cpp
#include "os.hpp"
#include "logger.hpp"

LOGGER("app")

// 共享数据
Mutex counterMutex("Counter");
int counter = 0;
EventGroup events("AppEvents");

#define EVENT_DATA_READY  (1 << 0)
#define EVENT_SHUTDOWN    (1 << 1)

// 生产者任务
class Producer : public Worker {
public:
    void run() override {
        while (true) {
            // 更新数据
            counterMutex.lock(TIMEOUT_FOREVER);
            counter++;
            counterMutex.unlock();
            
            // 通知消费者
            events.set(EVENT_DATA_READY);
            
            os::sleep(100);
        }
    }
};

// 消费者任务
class Consumer : public Worker {
public:
    void run() override {
        while (true) {
            uint32_t actualFlags;
            Result rst = events.wait(
                EVENT_DATA_READY | EVENT_SHUTDOWN,
                actualFlags,
                EventOptions_WaitForAny | EventOptions_Clear,
                TIMEOUT_FOREVER
            );
            
            if (actualFlags & EVENT_SHUTDOWN) {
                LOG_I("Shutting down");
                break;
            }
            
            if (actualFlags & EVENT_DATA_READY) {
                counterMutex.lock(TIMEOUT_FOREVER);
                int value = counter;
                counterMutex.unlock();
                
                LOG_I("Counter: %d", value);
            }
        }
    }
};

// 主程序
Producer producer;
Consumer consumer;

Thread<2048> producerThread("Producer", &producer, 10);
Thread<2048> consumerThread("Consumer", &consumer, 10);

void startApp() {
    producerThread.start();
    consumerThread.start();
}

void stopApp() {
    events.set(EVENT_SHUTDOWN);
}
```

## 参考资源

- [ThreadX官方文档](https://docs.microsoft.com/en-us/azure/rtos/threadx/)
- [FreeRTOS官方文档](https://www.freertos.org/Documentation/RTOS_book.html)
- [wibotlib源码](../src/os/)

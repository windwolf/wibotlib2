# BootFrom 使用指南

## 概述

`BootFrom` 是 wibotlib 应用框架中的核心启动模板类，用于实现从 C 环境到 C++ 环境的平滑切换，并确保正确的初始化顺序。它在嵌入式系统的底层驱动初始化完成后、应用代码执行前提供了一个关键的桥接点。

## 核心目标

### 1. C/C++ 环境切换
从纯 C 的 HAL/RTOS 环境切换到 wibotlib 的 C++ 环境，处理：
- C++ 运行时环境的准备
- C++ 对象的构造和初始化
- C/C++ 混合编程的接口桥接

### 2. 初始化顺序控制
确保对象和库的初始化发生在正确的时机：
```
底层驱动初始化 (HAL/CubeMX) 
    ↓
BootFrom::run()  ← 【关键切换点】
    ↓
C++ 对象构造和库初始化
    ↓
应用程序代码执行 (app.run())
```

### 3. 其他特性
- **应用实例管理**：自动管理应用程序的全局单例实例
- **日志初始化**：集成日志系统的初始化和版本信息输出
- **调试支持**：可选的 RTT 打印功能支持
- **类型安全**：使用模板确保编译时类型检查

## 核心特性

### 1. C/C++ 环境桥接
通过 `extern "C"` 接口与 C 代码无缝集成，实现从 C 环境到 C++ 环境的切换：
- C 代码可以安全调用 C++ 应用
- 处理 C++ 全局对象的构造
- 确保 C++ 运行时环境就绪

### 2. 精确的初始化时机控制
在正确的时间点执行初始化：
```cpp
main()                          // C 环境
  └─> HAL_Init()               // STM32 HAL 初始化
  └─> MX_Peripheral_Init()     // 外设初始化（GPIO、Timer、UART等）
  └─> bootApp()                // ← 切换点
      └─> BootFrom<T>::run()  // C++ 环境开始
          └─> static T _app;  // C++ 对象构造
          └─> _app.run();     // 应用代码执行
```

### 3. 模板化设计
使用 C++ 模板参数化应用程序类型，在编译时确定应用类型，无运行时开销。

### 4. 全局实例访问
提供静态成员 `app` 指针，允许在应用程序的任何地方访问应用实例。

### 5. 统一初始化流程
- RTT 初始化（可选）
- 日志系统初始化
- 版本信息输出
- 应用实例创建
- 应用程序运行

## 类定义

### 位置
[libs/wibotlib/src/app/app-framework.hpp](../src/app/app-framework.hpp)

### 接口

```cpp
namespace wibot {

template <typename T>
class BootFrom {
   public:
    /// 应用程序全局实例指针
    static T *app;
    
    /**
     * @brief 启动应用程序
     * 
     * 此函数是从 C 环境切换到 C++ 环境的关键入口点。
     * 它会创建应用对象实例，初始化日志系统，然后调用应用的 run() 方法。
     * 
     * @param name 应用程序名称，用于日志输出
     * @param version 应用程序版本号，用于日志输出
     * 
     * @note 要求类型 T 必须有 run() 方法
     * @note 此函数通常不会返回（应用的 run() 包含主循环）
     */
    static void run(char const *name, char const *version);
};

}  // namespace wibot
```

## 使用方法

### 基础用法

#### 1. 定义应用程序类

应用程序类**只需要实现 `run()` 方法**，不强制要求继承特定基类。

**最简示例**：

```cpp
class MinimalApp {
public:
    void run() {
        // 初始化
        init();
        
        // 主循环
        while (true) {
            // 应用逻辑
        }
    }
    
private:
    void init() {
        // 初始化代码
    }
};
```

**使用 ControlLoop 的示例**（推荐）：

虽然 `BootFrom` 不依赖 `ControlLoop`，但推荐使用 `ControlLoop` 及其派生类来组织应用结构

```cpp
#include "app-framework.hpp"
#include "control-loop.hpp"
#include "logger.hpp"

LOGGER("app")

using namespace wibot;

class MyApp : public TimerControlLoop {
public:
    MyApp() : TimerControlLoop(htim16, 100) {
        // 构造函数：初始化定时器控制循环，频率100Hz
    }

    void init() override {
        // 应用程序初始化代码
        LOG_I("Application initializing...");
        
        // 初始化各个模块
        // ...
    }

    void doLoop() override {
        // 控制循环主体，会以指定频率被周期性调用
        // 执行周期性任务
        // ...
    }
};
```

#### 2. 创建启动函数（C/C++ 桥接点）

在 C++ 源文件中定义 `extern "C"` 启动函数，这是从 C 环境切换到 C++ 环境的关键接口：

```cpp
// app.cpp
extern "C" void bootApp() {
    // 此调用将：
    // 1. 初始化 C++ 运行时环境
    // 2. 构造 MyApp 对象
    // 3. 调用 MyApp::run()
    BootFrom<MyApp>::run("MyApp", "1.0.0");
}
```

**为什么需要 `extern "C"`？**
- 避免 C++ 名称修饰（name mangling）
- 使 C 代码能够直接调用此函数
- 建立 C/C++ 环境的桥接

#### 3. 从 C 代码调用（关键时机）

在 main.c 中，**必须在所有底层驱动初始化完成后**调用启动函数：

```c
/* main.c */
#include "app-framework.h"  // 声明 bootApp()

int main(void) {
    // ========== 阶段 1: 底层驱动初始化 (C 环境) ==========
    HAL_Init();              // STM32 HAL 初始化
    SystemClock_Config();    // 系统时钟配置
    
    // 外设初始化 - 这些必须在 bootApp() 之前完成
    MX_GPIO_Init();          // GPIO 初始化
    MX_DMA_Init();           // DMA 初始化
    MX_I2C_Init();           // I2C 初始化
    MX_UART_Init();          // UART 初始化
    MX_TIM_Init();           // Timer 初始化
    // ...
    
    // ========== 阶段 2: 切换到 C++ 环境 ==========
    bootApp();  // ← 【关键切换点】不会返回
    
    // 不应该执行到这里
    while (1) { }
}
```

### AQ-BF01 项目实战示例

以下是 AQ-BF01 充电器控制系统的完整实现示例：

#### 应用程序类定义

```cpp
// App/app.cpp
#include "app-framework.hpp"
#include "control-loop.hpp"
#include "charger.hpp"
#include "esc.hpp"
#include "led.hpp"
#include "light.hpp"
#include "power.hpp"
#include "key-scaner.hpp"
#include "config.hpp"

#include "logger.hpp"
LOGGER("app")

using namespace wibot;

class App : public TimerControlLoop {
public:
    enum class AppStatus {
        kRunning,
        kShutdowning,
        kShutdown,
    };

    App() : TimerControlLoop(htim16, CONTROL_LOOP_FREQUENCY) {
        // 初始化按键引脚配置
        keyPinConfigs[0] = {I_KEY_ONOFF_GPIO_Port, I_KEY_ONOFF_Pin};
        keyPinConfigs[1] = {I_KEY_INC_GPIO_Port, I_KEY_INC_Pin};
        keyPinConfigs[2] = {I_KEY_DEC_GPIO_Port, I_KEY_DEC_Pin};

        // 配置 GPIO
        GpioDigitalSourceConfig config{
            .digitalConfig = {.inverse = 0x00000111, .debounceTimeMs = 20},
            .pins = keyPinConfigs,
            .pinCount = 3
        };
        gpio.configureGpio(config);
    }

    void init() override {
        // 初始化各个功能模块
        power.init();
        charger.init();
        
        if constexpr (APPMODE == AppMode::kNormal) {
            light.on();
            esc.off();
        }
    }

    void doLoop() override {
        if constexpr (APPMODE == AppMode::kChargerTest) {
            // 充电器测试模式
            static u32 loopCount = 0;
            if (loopCount++ % CONTROL_LOOP_FREQUENCY == 0) {
                charger.updateInfo();
                charger.feedWatchdog();
                auto status = charger.getStatus();
                LOG_I("Charger status: %d", static_cast<int>(status));
            }
            
        } else if constexpr (APPMODE == AppMode::kNormal) {
            // 正常工作模式
            if (status == AppStatus::kRunning) {
                // 电源管理
                auto ar = power.beginUpdate();
                
                // 充电管理
                charger.updateInfo();
                charger.feedWatchdog();
                
                // 按键扫描
                keyScaner.scan();
                
                // 按键处理
                if (keyScaner.hasEvent(0, KeyEvent::kLongPress)) {
                    LOG_I("ONOFF long pressed. Shutdowning...");
                    status = AppStatus::kShutdowning;
                    shutdownTick = System::getTick();
                }
                
                if (keyScaner.hasEvent(1, KeyEvent::kClick)) {
                    light.toggle();
                    LOG_I("Light toggled: %d", light.isOn());
                }
                
                if (keyScaner.hasEvent(2, KeyEvent::kClick)) {
                    esc.inc();
                    LOG_I("ESC increased. Level: %d", esc.getLevel());
                }
                
                if (keyScaner.hasEvent(3, KeyEvent::kClick)) {
                    esc.dec();
                    LOG_I("ESC decreased. Level: %d", esc.getLevel());
                }
                
                // 更新电机和电源
                esc.update();
                power.endUpdate(ar);
                
                // 更新 LED 指示
                auto pwrLevel = power.getBatteryLevel();
                auto escLevel = esc.getLevel();
                led.setPowerLevel(pwrLevel);
                led.setEscLevel(escLevel);
                led.update();
                
                LOG_I_INTERVAL(CONTROL_LOOP_FREQUENCY / 2, 
                              "batt: %d, esc: %d", pwrLevel, escLevel);
            }
            else if (status == AppStatus::kShutdowning) {
                // 关机流程
                LOG_I("System shutdowning...");
                light.off();
                esc.off();
                led.setEscLevel(0);
                led.update();
                esc.update();
                
                if (System::getDurationMs(shutdownTick) > 2000) {
                    status = AppStatus::kShutdown;
                }
            }
            else if (status == AppStatus::kShutdown) {
                // 关闭电源
                power.shutdown();
            }
        }
    }

private:
    // GPIO 引脚配置
    GpioPinConfig keyPinConfigs[3];
    
    // 功能模块实例
    GpioDigitalSource<3> gpio{GpioDigitalSourceConfig{}};
    KeyScaner<3> keyScaner{gpio, KeyScanerConfig{
        .holdThreshold = 3000,
        .clickIntervalThreshold = 500,
    }};
    
    Power power;
    Esc esc;
    Charger charger;
    Led led;
    Light light;
    
    AppStatus status = AppStatus::kRunning;
    u32 shutdownTick = 0;
};

// 启动函数
extern "C" void bootApp() { 
    BootFrom<App>::run("AQ-BF01", "0.0.1"); 
}
```

#### C 语言启动集成

```c
/* Core/Src/main.c */
#include "main.h"
#include "app-framework.h"  // C 头文件
#include "app_threadx.h"

int main(void) {
    // 系统初始化
    HAL_Init();
    SystemClock_Config();
    
    // 外设初始化
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_I2C2_Init();
    MX_SPI3_Init();
    MX_USART2_UART_Init();
    MX_ADC2_Init();
    MX_TIM16_Init();
    
    // 启动 RTOS
    MX_ThreadX_Init();  // 最终会调用 bootApp()
    
    // 不应该执行到这里
    while (1) { }
}
```

```c
/* AZURE_RTOS/App/app_azure_rtos.c */
#include "app_azure_rtos.h"
#include "app-framework.h"

VOID tx_application_define(VOID *first_unused_memory) {
    // ThreadX 初始化
    // ...
    
    // 在适当的位置调用应用启动函数
    bootApp();  // 启动应用程序，不会返回
}
```

## 工作流程

### 启动序列

```
【C 环境 - 底层驱动层】
1. main() 函数
   ├─> HAL_Init()                    // STM32 HAL 初始化
   ├─> SystemClock_Config()          // 时钟配置
   ├─> MX_GPIO_Init()                // GPIO 初始化
   ├─> MX_DMA_Init()                 // DMA 初始化  
   ├─> MX_I2C_Init()                 // I2C 初始化
   ├─> MX_UART_Init()                // UART 初始化
   ├─> MX_TIM_Init()                 // Timer 初始化
   └─> MX_ThreadX_Init() [可选]      // RTOS 初始化
       └─> tx_application_define()

【切换点 - C/C++ 桥接】
           └─> bootApp()              // extern "C" 函数

【C++ 环境 - wibotlib 层】
               └─> BootFrom<App>::run("AppName", "Version")
                   ├─> [可选] SEGGER_RTT_Init()     // RTT 调试初始化
                   ├─> LOGGER("app")                // 日志系统初始化
                   ├─> LOG_I("App name: ...")       // 输出版本信息
                   ├─> static App _app;             // ← C++ 对象构造
                   ├─> app = &_app;                 // 设置全局指针
                   └─> _app.run()                   // ← 调用应用的 run() 方法

【应用层 - 用户代码】
                       └─> 应用程序逻辑
                           ├─> 初始化
                           └─> 主循环（实现方式由应用决定）
```

**关键点说明**：

1. **底层驱动初始化（C 环境）**：HAL、外设、DMA、定时器等必须先完成
2. **环境切换点**：`bootApp()` 是 C/C++ 的桥接点
3. **C++ 对象构造**：`static App _app` 在此时才被构造
4. **应用代码执行**：`_app.run()` 执行应用逻辑，通常包含主循环

### 应用类的 run() 方法实现

`BootFrom` 只要求应用类有 `run()` 方法，具体实现由应用决定。

#### 方式 1: 简单循环

```cpp
class SimpleApp {
public:
    void run() {
        init();
        while (true) {
            doWork();
            HAL_Delay(100);
        }
    }
};
```

#### 方式 2: 使用 ControlLoop（推荐）

wibotlib 提供了 `ControlLoop` 框架来简化循环控制：

```cpp
class App : public TimerControlLoop {
public:
    App() : TimerControlLoop(htim16, 100) {}  // 100Hz
    
    void init() override {
        // 初始化代码
    }
    
    void doLoop() override {
        // 周期性执行的代码
    }
    
    // run() 方法由 ControlLoop 基类提供：
    // void run() {
    //     init();
    //     while (true) {
    //         doLoop();
    //         waitForTimer();
    //     }
    // }
};
```

#### 方式 3: RTOS 任务创建

```cpp
class RtosApp {
public:
    void run() {
        // 创建 RTOS 任务
        tx_thread_create(&thread, "app", threadEntry, ...);
        // 启动调度器或返回
    }
    
private:
    static void threadEntry(ULONG arg) {
        while (true) {
            // 任务逻辑
        }
    }
    TX_THREAD thread;
};
```

**关键点**：
- `run()` 方法通常包含主循环，因此不会返回
- 可以使用任何循环控制方式：轮询、定时器、RTOS、事件驱动等
- `ControlLoop` 是推荐方式，但不是必须的

## 全局实例访问

通过 `BootFrom<T>::app` 静态成员访问应用实例：

```cpp
// 在任何需要访问应用实例的地方
extern wibot::BootFrom<App> bootFromApp;

void someFunction() {
    if (BootFrom<App>::app != nullptr) {
        // 访问应用实例
        auto level = BootFrom<App>::app->power.getBatteryLevel();
    }
}
```

**注意**：
- 在 `BootFrom::run()` 调用之前，`app` 指针为 `nullptr`
- 只在应用完全启动后访问 `app` 指针

## 可选功能配置

### RTT 打印支持

在项目中定义 `USE_RTT_PRINT` 宏启用 SEGGER RTT 支持：

```cmake
# CMakeLists.txt
target_compile_definitions(${PROJECT_NAME} PRIVATE
    USE_RTT_PRINT
)
```

启用后，`BootFrom::run()` 会自动初始化 RTT。

## 最佳实践

### 1. 确保正确的初始化顺序

**最重要**：必须在 `bootApp()` 调用前完成所有底层驱动初始化。

```c
// ✅ 正确：底层驱动先初始化
int main(void) {
    HAL_Init();
    MX_GPIO_Init();  // GPIO 必须先初始化
    MX_TIM_Init();   // Timer 必须先初始化
    bootApp();       // 然后启动 C++ 应用
}

// ❌ 错误：在 C++ 对象构造中初始化外设
class App {
public:
    App() {
        MX_GPIO_Init();  // 太晚了！应该在 bootApp() 之前
    }
};
```

### 2. 应用类设计

应用类需要有 `run()` 方法，其他设计由你决定：

```cpp
// 选项 A: 不继承任何类
class App {
public:
    void run() {
        init();
        while (true) { /* ... */ }
    }
};

// 选项 B: 继承 ControlLoop（推荐）
class App : public TimerControlLoop {
public:
    App() : TimerControlLoop(htim, freq) {}
    void init() override { /* ... */ }
    void doLoop() override { /* ... */ }
    // run() 由基类提供
};
```

**设计原则**：
- **单一职责**：应用类只负责协调各个功能模块
- **模块化**：将功能拆分为独立的模块类
- **资源管理**：使用 RAII 管理资源（在构造函数中初始化成员对象）

```cpp
class App : public TimerControlLoop {
private:
    // 成员对象在 App 构造时自动构造
    // 这发生在 BootFrom::run() 中的 "static App _app;"
    PowerManager power;      // ← 此时 GPIO/ADC 已初始化
    MotorController motor;   // ← 此时 Timer/PWM 已初始化
    UartComm comm;           // ← 此时 UART 已初始化
};
```

### 2. 启动函数命名

使用清晰的命名约定：

```cpp
// 推荐
extern "C" void bootApp() { }

// 或者更具体的名称
extern "C" void bootMyApplication() { }
```

### 3. 版本管理

使用配置文件管理版本信息：

```cpp
// config.hpp
#define APP_NAME    "AQ-BF01"
#define APP_VERSION "0.0.1"

// app.cpp
extern "C" void bootApp() { 
    BootFrom<App>::run(APP_NAME, APP_VERSION); 
}
```

### 4. 日志使用

在应用类文件开头声明日志标签：

```cpp
#include "logger.hpp"
LOGGER("app")  // 定义日志标签

class App : public TimerControlLoop {
    void init() override {
        LOG_I("Initializing application...");
    }
};
```

### 5. 控制循环频率选择

根据应用需求选择合适的频率：

```cpp
// 低速控制（传感器采样、LED 控制）
TimerControlLoop(htim, 10);   // 10 Hz

// 中速控制（按键扫描、状态机）
TimerControlLoop(htim, 50);   // 50 Hz

// 高速控制（电机控制、实时通信）
TimerControlLoop(htim, 1000); // 1 kHz
```

## 常见问题

### Q1: 为什么 bootApp() 不返回？

`BootFrom::run()` 内部调用 `_app.run()`，而应用的 `run()` 方法通常包含主循环：

```cpp
class App {
public:
    void run() {
        init();
        while (true) {  // 无限循环
            // 应用逻辑
        }
    }
};

// 或者使用 ControlLoop：
void ControlLoop::run() {
    init();
    auto ar = getLoopSignal();
    while (true) {  // 无限循环
        doLoop();
        ar.wait(TIMEOUT_FOREVER);
    }
}
```

### Q2: 如何在不同的 RTOS 中使用？

`BootFrom` 与 RTOS 无关，可以在以下环境中使用：

- **ThreadX**：在 `tx_application_define()` 或主线程中调用
- **FreeRTOS**：在主任务中调用
- **No-OS**：在 `main()` 函数中调用

### Q3: 为什么使用 `static T _app` 而不是 `new T()`？

**初始化顺序保证**：使用静态局部变量确保对象在正确的时机构造：

```cpp
static void run(char const *name, char const *version) {
    // 此时所有底层驱动已初始化完成
    static T _app;  // ← 在这里构造，时机可控
    app = &_app;
    _app.run();
}
```

如果使用全局对象：
```cpp
App globalApp;  // ❌ 构造时机不确定，可能在 main() 之前

int main() {
    HAL_Init();     // 太晚了！globalApp 已经构造了
    MX_GPIO_Init();
}
```

**可以创建多个应用实例吗？**  
不建议。`BootFrom` 设计为单例模式，多次调用 `run()` 会创建多个实例，但只有最后一个会被 `app` 指针引用。

### Q4: 如何处理初始化错误？

在 `init()` 方法中检查并处理错误：

```cpp
void init() override {
    if (power.init() != Result::kOk) {
        LOG_E("Power initialization failed!");
        // 错误处理：进入安全模式或停机
        while (1) { }
    }
    
    if (motor.init() != Result::kOk) {
        LOG_E("Motor initialization failed!");
        // 错误处理
    }
}
```

## 相关文档

- [ControlLoop 控制循环文档](CONTROL_LOOP_USAGE.md)
- [Logger 日志系统文档](LOGGER_USAGE.md)
- [Async 异步编程文档](ASYNC_USAGE.md)
- [CONSTRUCTOR_CONTROL_USAGE.md](CONSTRUCTOR_CONTROL_USAGE.md)：构造函数执行顺序控制

## 总结

`BootFrom` 的核心价值在于提供从 C 到 C++ 的安全桥接和精确的初始化顺序控制：

✅ **C/C++ 环境切换**：从 HAL/C 环境平滑切换到 wibotlib/C++ 环境  
✅ **初始化顺序保证**：确保底层驱动初始化 → C++ 对象构造 → 应用代码执行  
✅ **类型安全**：模板提供编译时类型检查  
✅ **无依赖限制**：只要求应用类有 `run()` 方法，不强制继承特定基类  
✅ **调试友好**：集成日志和版本信息输出  

**关键理解**：
```
底层驱动 (C/HAL)  →  BootFrom (桥接)  →  应用对象 (C++)  →  应用逻辑
   [GPIO/I2C/Timer]    [环境切换]      [对象构造]        [run()]
```

通过正确使用 `BootFrom`，可以避免常见的初始化顺序问题，构建稳定可靠的嵌入式应用程序。

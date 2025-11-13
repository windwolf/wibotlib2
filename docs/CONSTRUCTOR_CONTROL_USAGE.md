# C++ 构造函数执行顺序控制使用指南

本文档说明如何使用 `wibot::Initializer` 框架中集成的构造函数优先级控制功能。

## 概述

通过 GNU C++ 的 `init_priority` 属性和自定义链接脚本，我们可以精确控制全局对象构造函数的执行顺序。

## 前置条件

1. **链接器脚本**: 确保项目已包含 `constructor_manual_init.ld` 补充链接脚本
2. **CMake 配置**: 在 `cmake/gcc-arm-none-eabi.cmake` 中添加:
   ```cmake
   target_link_options(${CMAKE_PROJECT_NAME}.elf PRIVATE
       -T "${CMAKE_SOURCE_DIR}/constructor_manual_init.ld"
   )
   ```

## 可用的宏

### 自动初始化宏（按优先级从高到低）

```cpp
HAL_CONSTRUCTOR          // 优先级 101 - HAL 层驱动
DRIVER_CONSTRUCTOR       // 优先级 200 - 设备驱动
MIDDLEWARE_CONSTRUCTOR   // 优先级 500 - 中间件
APPLICATION_CONSTRUCTOR  // 优先级 1000 - 应用层对象
CUSTOM_PRIORITY(n)       // 自定义优先级 (101-65535)
```

### 手动初始化宏

```cpp
MANUAL_INIT              // 延迟到手动调用时才构造
MANUAL_INIT_PRIORITY(n)  // 延迟构造，但指定内部顺序
```

## 使用方法

### 1. 自动初始化（带优先级）

**重要限制**: `init_priority` 属性只能应用于**变量声明**，不能应用于类定义。

#### ✅ 正确用法

```cpp
// 在对象声明时使用宏
class MyDriver {
public:
    MyDriver() {
        // 初始化硬件
    }
};

// HAL 层对象，优先级最高
MyDriver HAL_CONSTRUCTOR halDriver;

// 依赖 HAL 的驱动对象
MyDriver DRIVER_CONSTRUCTOR deviceDriver;

// 应用层对象，最后初始化
MyDriver APPLICATION_CONSTRUCTOR appObject;
```

#### ❌ 错误用法（GCC 不支持）

```cpp
// 错误：不能在类定义上使用 init_priority
class HAL_CONSTRUCTOR MyDriver {  // 编译错误！
    // ...
};
```

### 2. 手动初始化（延迟构造）

对于需要在特定时机才能初始化的对象（例如文件系统、网络栈等），使用 `MANUAL_INIT`：

```cpp
class FileSystem {
public:
    FileSystem() {
        // 需要在 SD 卡就绪后才能初始化
    }
};

// 这个对象的构造函数不会自动执行
FileSystem MANUAL_INIT g_filesystem;
```

在合适的时机手动触发初始化：

```cpp
int main() {
    // 自动初始化的对象已完成构造
    
    // 初始化 SD 卡
    sdcard_init();
    
    // 现在触发手动初始化的对象
    wibot::InitManager::executeManualInit();
    
    // g_filesystem 现在可以使用了
    while(1) {
        // ...
    }
}
```

### 3. 与 Initializable 框架结合

`wibot::Initializer` 提供的运行时注册机制可以与构造函数优先级控制结合使用：

```cpp
class MyComponent : public wibot::Initializable {
public:
    // 构造函数在指定优先级执行
    MyComponent() {
        // 基础构造
        // 自动注册到 Initializer
        wibot::Initializer::getInstance().registerInitialObject(this);
    }
    
    // 进一步的初始化逻辑
    void _init() override {
        // 在 initOnBoot() 被调用时执行
    }
};

// 这个对象将在 DRIVER 阶段构造
// 但 _init() 方法要等到 initOnBoot() 被调用
MyComponent DRIVER_CONSTRUCTOR g_component;
```

## 典型初始化流程

```cpp
// 启动流程：
//
// 1. startup_stm32g431xx.s 中的 Reset_Handler
// 2. __libc_init_array() - 调用自动初始化的构造函数
//    2.1 HAL_CONSTRUCTOR 对象 (优先级 101)
//    2.2 DRIVER_CONSTRUCTOR 对象 (优先级 200)
//    2.3 MIDDLEWARE_CONSTRUCTOR 对象 (优先级 500)
//    2.4 APPLICATION_CONSTRUCTOR 对象 (优先级 1000)
//    2.5 无优先级的普通对象 (优先级 65535)
// 3. main() 函数
// 4. wibot::Initializer::getInstance().initOnBoot() - 调用 _init()
// 5. [用户代码] 手动调用 InitManager::executeManualInit()
// 6. MANUAL_INIT 对象的构造函数被执行
// 7. 应用主循环

int main() {
    // 阶段1: 自动初始化已完成
    
    // 阶段2: 运行时初始化
    auto& init = wibot::Initializer::getInstance();
    init.initOnBoot(wibot::InitialableType::normal);
    
    // 阶段3: 手动初始化
    wibot::InitManager::executeManualInit();
    init.initOnBoot(wibot::InitialableType::os);
    init.finishInit();
    
    // 阶段4: 应用运行
    while(1) {
        // ...
    }
}
```

## 实际应用示例

### 示例 1: 分层驱动初始化

```cpp
// hal_gpio.hpp
class GPIO {
public:
    GPIO() { /* 配置引脚 */ }
};
GPIO HAL_CONSTRUCTOR g_gpio;

// driver_sensor.hpp
class Sensor {
public:
    Sensor() { 
        // 依赖 g_gpio，确保后初始化
    }
};
Sensor DRIVER_CONSTRUCTOR g_sensor;

// app_controller.hpp
class Controller {
public:
    Controller() {
        // 依赖 g_sensor
    }
};
Controller APPLICATION_CONSTRUCTOR g_controller;
```

### 示例 2: 延迟初始化的文件系统

```cpp
// filesystem.hpp
class FileSystem {
private:
    bool mounted_ = false;
public:
    FileSystem() {
        // 尝试挂载 SD 卡
        if (sdcard_detect()) {
            mount();
            mounted_ = true;
        }
    }
};

// 延迟到 SD 卡检测完成后再构造
FileSystem MANUAL_INIT g_fs;

// main.cpp
int main() {
    // 等待 SD 卡插入
    while (!sdcard_detect()) {
        delay(100);
    }
    
    // 现在初始化文件系统
    wibot::InitManager::executeManualInit();
    
    // 使用文件系统
    // ...
}
```

## 调试技巧

### 检查初始化顺序

可以在构造函数中添加日志来验证执行顺序：

```cpp
class DebugObject {
public:
    DebugObject(const char* name) {
        printf("Constructing: %s\n", name);
    }
};

DebugObject HAL_CONSTRUCTOR obj1("HAL Object");
DebugObject DRIVER_CONSTRUCTOR obj2("Driver Object");
DebugObject APPLICATION_CONSTRUCTOR obj3("App Object");
DebugObject MANUAL_INIT obj4("Manual Object");
```

### 查看链接器生成的符号

```bash
arm-none-eabi-nm build/Debug/wibotlib3_t4.elf | grep init_array
```

输出示例：
```
08001234 T __init_array_start
08001240 T __init_array_end
08001250 T __init_array_manual_start
08001258 T __init_array_manual_end
```

## 注意事项

1. **优先级范围**: 101-65535，数值越小优先级越高
2. **保留优先级**: 1-100 由编译器保留（用于 C++ 标准库）
3. **属性限制**: `init_priority` 只能用于全局/静态变量声明，不能用于类定义
4. **手动初始化幂等性**: `executeManualInit()` 可安全多次调用，只执行一次
5. **线程安全**: 构造函数在单线程环境中执行，无需考虑竞态条件
6. **依赖关系**: 确保高优先级对象不依赖低优先级对象

## 常见问题

### Q: 为什么不能在类定义上使用宏？

A: GCC 的 `init_priority` 属性只能应用于变量声明，这是编译器的限制。需要在每个对象声明时指定优先级。

### Q: 多次调用 executeManualInit() 会重复构造吗？

A: 不会。`executeManualInit()` 内部使用静态布尔标志确保只执行一次。

### Q: 可以混合使用优先级和手动初始化吗？

A: 可以。自动初始化的对象在 `main()` 之前构造，手动初始化的对象在调用 `executeManualInit()` 时构造。

### Q: 如何处理初始化失败？

A: 在构造函数中设置状态标志，并在后续代码中检查：

```cpp
class Driver {
private:
    bool initialized_ = false;
public:
    Driver() {
        if (hardware_init() == OK) {
            initialized_ = true;
        }
    }
    bool isReady() const { return initialized_; }
};

Driver DRIVER_CONSTRUCTOR g_driver;

int main() {
    if (!g_driver.isReady()) {
        error_handler();
    }
}
```

## 参考资料

- [GCC 属性文档 - init_priority](https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Attributes.html)
- [GNU LD 手册 - INSERT 命令](https://sourceware.org/binutils/docs/ld/Miscellaneous-Commands.html)
- STM32G431xx 启动文件: `startup_stm32g431xx.s`
- 链接器脚本: `STM32G431XX_FLASH.ld`, `constructor_manual_init.ld`

# 构造函数优先级控制 - 快速参考

## 可用宏

```cpp
// 自动初始化（按优先级）
HAL_CONSTRUCTOR          // 优先级 101
DRIVER_CONSTRUCTOR       // 优先级 200  
MIDDLEWARE_CONSTRUCTOR   // 优先级 500
APPLICATION_CONSTRUCTOR  // 优先级 1000
CUSTOM_PRIORITY(n)       // 自定义 (101-65535)

// 手动初始化
MANUAL_INIT              // 延迟初始化
MANUAL_INIT_PRIORITY(n)  // 延迟+内部顺序
```

## 使用语法

### ✅ 正确（应用于变量）
```cpp
MyClass HAL_CONSTRUCTOR obj1;
MyClass DRIVER_CONSTRUCTOR obj2;
MyClass MANUAL_INIT obj3;
```

### ❌ 错误（不能应用于类）
```cpp
class HAL_CONSTRUCTOR MyClass { };  // 编译错误！
```

## 执行手动初始化

```cpp
#include "initial.hpp"

int main() {
    // 触发延迟构造
    wibot::InitManager::executeManualInit();
}
```

## 完整初始化流程

```cpp
int main() {
    // 1. HAL/DRIVER/APP 对象已自动构造完成
    
    // 2. 运行时初始化
    auto& init = wibot::Initializer::getInstance();
    init.initOnBoot(wibot::InitialableType::normal);
    
    // 3. 手动触发延迟构造
    wibot::InitManager::executeManualInit();
    init.initOnBoot(wibot::InitialableType::os);
    init.finishInit();
    
    // 4. 应用主循环
    while(1) { }
}
```

## 优先级规则

- 数值越小，越早执行
- 范围: 101-65535 (1-100 系统保留)
- 无优先级对象 = 65535

## 典型使用场景

| 场景            | 使用宏                  | 原因           |
| --------------- | ----------------------- | -------------- |
| GPIO/时钟初始化 | HAL_CONSTRUCTOR         | 最先执行       |
| SPI/I2C 设备    | DRIVER_CONSTRUCTOR      | 依赖 HAL       |
| RTOS 任务创建   | MIDDLEWARE_CONSTRUCTOR  | 依赖驱动       |
| 应用逻辑对象    | APPLICATION_CONSTRUCTOR | 最后初始化     |
| 文件系统/网络   | MANUAL_INIT             | 需要运行时条件 |

## 调试验证

```cpp
class Test {
public:
    Test(const char* name) { 
        printf("Init: %s\n", name); 
    }
};

Test HAL_CONSTRUCTOR t1("HAL");
Test DRIVER_CONSTRUCTOR t2("Driver");
Test APPLICATION_CONSTRUCTOR t3("App");
Test MANUAL_INIT t4("Manual");
```

输出顺序:
```
Init: HAL
Init: Driver
Init: App
[main() 执行]
[调用 executeManualInit()]
Init: Manual
```

## 配置检查清单

- [ ] `constructor_manual_init.ld` 存在于项目根目录
- [ ] `cmake/gcc-arm-none-eabi.cmake` 包含补充链接脚本
- [ ] `#include "wibotlib/src/base/initial.hpp"` 
- [ ] 在 main() 中调用 `InitManager::executeManualInit()`

## 常见错误

| 错误                                                          | 原因                       | 解决方法             |
| ------------------------------------------------------------- | -------------------------- | -------------------- |
| 编译错误: 'init_priority' attribute only applies to variables | 宏用在了类定义上           | 移到变量声明         |
| 链接错误: undefined reference to __init_array_manual_start    | 未包含补充链接脚本         | 检查 CMake 配置      |
| 对象未构造                                                    | executeManualInit() 未调用 | 在 main() 中添加调用 |

## 完整示例

```cpp
// driver.hpp
class SensorDriver {
public:
    SensorDriver() { /* 初始化传感器 */ }
    float read() { /* 读取数据 */ }
};

// 驱动对象，优先级 200
SensorDriver DRIVER_CONSTRUCTOR g_sensor;

// filesystem.hpp  
class FileSystem {
public:
    FileSystem() { /* 挂载 SD 卡 */ }
};

// 延迟构造（等待 SD 卡插入）
FileSystem MANUAL_INIT g_fs;

// main.cpp
#include "initial.hpp"

int main() {
    // g_sensor 已经可用
    float data = g_sensor.read();
    
    // 等待 SD 卡就绪
    while (!sd_ready()) delay(100);
    
    // 现在初始化文件系统
    wibot::InitManager::executeManualInit();
    
    // g_fs 现在可用
    while(1) { }
}
```

详细文档: `CONSTRUCTOR_CONTROL_USAGE.md`

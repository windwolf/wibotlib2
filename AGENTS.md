# WibotLib - STM32嵌入式开发库 - AI助手指南

## 概述

WibotLib是一个为STM32系列MCU设计的现代C++嵌入式开发库，提供完整的硬件抽象层、实时操作系统支持和丰富的设备驱动。本库采用模块化设计，支持多种RTOS，并提供统一的API接口，极大地简化了嵌入式系统开发。

## 库的架构

### 分层架构

```
┌─────────────────────────────────────────────┐
│             应用层 (App Layer)              │
├─────────────────────────────────────────────┤
│      设备驱动层 (Device Driver Layer)       │
│     显示│存储│电机│射频│时钟│IO│传感器      │
├─────────────────────────────────────────────┤
│          协议层 (Protocol Layer)            │
│     GNSS │ Camera │ DShot │ Modbus          │
├─────────────────────────────────────────────┤
│       数据模型层 (Data Model Layer)         │
│ Source│Filter│Mapper│Adapter│Controller│RW  │
├─────────────────────────────────────────────┤
│    硬件抽象层 (HAL - Hardware Abstraction)  │
│    GPIO │ SPI │ I2C │ UART │ ADC │ Timer    │
├─────────────────────────────────────────────┤
│      操作系统移植层 (OS Port Layer)         │
│     FreeRTOS │ ThreadX │ No-OS              │
├─────────────────────────────────────────────┤
│         基础组件 (Base Components)          │
│  Types│Buffer│Chrono│Math│Peripheral│Async  │
└─────────────────────────────────────────────┘
```

## 目录结构

```
root/
├── src/                          # 主要源代码
│   ├── base/                     # 基础组件（类型、缓冲、时间、数学）
│   ├── os/                       # 操作系统抽象层
│   ├── hal/                      # 硬件抽象层（含stm32平台驱动）
│   ├── pp/                       # 数据流处理模型（source、filter、mapper、controller等）
│   ├── comm/                     # 通信和协议层（protocol、crc、modbus、msg）
│   ├── device/                   # 设备驱动（display、storage、motor、rf、rtc、io、rls、misc）
│   ├── app/                      # 应用框架
│   ├── graph/                    # 图形系统
│   ├── dsp/                      # 数字信号处理
│   ├── math/                     # 数学库（扩展）
│   ├── log/                      # 日志系统
│   ├── port/                     # 平台移植层
│   ├── old/                      # 过期代码（备用）
│   ├── boot.h                    # 启动定义
│   └── .clang-format             # Clang-format配置
│
├── docs/                         # 使用文档和指南
├── example/                      # 示例代码
├── test/                         # 单元测试
├── cmake/                        # CMake配置文件
├── CMakeLists.txt               # 主构建配置
├── README.md                    # 库说明文档
├── AGENTS.md                    # AI助手指南（本文件）
├── LICENSE                      # MIT许可证
└── .gitignore                   # Git忽略列表
```

## 核心功能特性

### 1. 多RTOS支持

- 完整支持FreeRTOS和ThreadX实时操作系统
- 统一的操作系统抽象层，可无缝切换不同RTOS
- 支持裸机（No-OS）运行模式

### 2. 现代异步编程模型

- 基于EventGroup的异步处理机制
- AsyncSource/AsyncResult模式优雅处理异步操作
- 资源池管理，避免内存碎片

### 3. 数据流Pipeline架构

- 基于SyncPipeline的数据处理框架
- 可链式组合的数据源、滤波器、映射器和控制器
- 支持多通道并行处理

### 4. 丰富的硬件抽象层

- 完整的STM32系列芯片硬件抽象
- 统一的外设接口，简化硬件操作
- 支持DMA、中断等高级特性

### 5. 全面的设备驱动支持

- **显示设备**: SSD1306、ST7735、ST77xx系列OLED/TFT显示屏
- **通信模块**: LoRa无线模块（Rola-E22）
- **传感器**: 多通道ADC传感器支持
- **存储设备**: Flash、EEPROM等存储组件
- **实时时钟**: RX8010等RTC芯片

### 6. 丰富的协议栈

- **GNSS定位**: 支持NMEA、UBX、CASIC等GNSS协议
- **摄像头接口**: OV7725等摄像头模块支持
- **电机控制**: DShot协议支持
- **Modbus**: Modbus主从站协议
- **CRC校验**: 多种CRC算法支持
- **消息系统**: 统一的消息定义和编解码框架

## 代码规范

### 编码风格

- 使用UTF-8编码
- C++文件使用`.cpp`扩展名，头文件使用`.hpp`扩展名，模板实现使用`.tpp`扩展名
- 头文件和实现文件放在同一目录下
- 遵循Google C++ Style Guide进行代码风格规范
- 使用Doxygen风格的注释进行代码文档编写

### 命名约定

- 类名使用PascalCase（如：`AdcChannel`）
- 函数名使用camelCase（如：`getValue()`）
- 变量名使用camelCase（如：`adcValue`）
- 类私有成员变量和函数使用前缀`_`（如：`_config`）
- 枚举类型使用PascalCase，枚举值使用kUpperCase（如：`Color { kRed, kGreen, kBlue }`）
- 常量使用UPPER_CASE或kConstantName
- 命名空间：所有代码都在`wibot`命名空间下
- 类的内部配置结构体统一命名为`Config`, 外部配置类以`XXXConfig`命名.

### 包含文件规则

- source文件和header文件使用相对路径包含
- 系统头文件使用`<>`包含
- 项目头文件使用`""`包含

### 类型系统

库定义了统一的类型系统（`base/type.hpp`）：

- `u8`, `u16`, `u32`, `u64`: 无符号整数
- `s8`, `s16`, `s32`, `s64`: 有符号整数
- `f32`, `f64`: 浮点数
- `Result`: 统一的错误处理机制

### 运行时配置管理

- 如果类的实现需要配置, 那么优先在类内部设置配置结构体.
- 需要配置的情况下, 配置结构体应作为引用传入构造函数, 避免拷贝开销.
- 配置结构体成员变量应设置默认值, 以简化使用.
- 提供`setConfig()`方法以便运行时修改配置.
- 提供`getConfig()`方法以便查询当前配置. 配置使用引用返回.
- 当外部修改了引用的内部配置后,提供`applyConfig()`方法以便应用配置更改.

## 构建系统

### CMake集成

WibotLib与STM32CubeMX生成的CMake项目无缝集成。使用时：

```cmake
# 添加WibotLib子目录
add_subdirectory(libs/wibotlib)

# 链接WibotLib到项目
target_link_libraries(${CMAKE_PROJECT_NAME}
    wibotlib_impl     # WibotLib实现
    wibotlib          # WibotLib接口
    stm32cubemx       # STM32CubeMX生成的代码
)
```

### 编译要求

- **C++标准**: C++23
- **C标准**: C11
- **CMake版本**: 3.22+
- **工具链**: ARM GCC或Clang
- **芯片支持**: STM32G4系列（其他系列持续扩展中）

### 构建步骤

在项目根目录下执行：

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Debug
```

## 使用指南

### 基本使用流程

1. **包含头文件**

```cpp
#include "wibotlib.hpp"  // 包含所有核心功能
```

1. **初始化系统**

```cpp
wibot::System::init();
```

1. **创建线程**

```cpp
class MyWorker : public wibot::Worker {
public:
    void run() override {
        while (true) {
            // 工作逻辑
            wibot::os::sleep(100);
        }
    }
};

MyWorker worker;
wibot::Thread<1024> thread("worker", &worker, 5);
thread.start();
```

1. **使用设备驱动**

```cpp
// SSD1306 OLED显示屏
wibot::SSD1306 display(i2c, 0x3C);
display.init();
display.print("Hello WibotLib!");
display.refresh();

// LoRa通信模块
wibot::RolaE22 lora(uart, gpio_m0, gpio_m1);
lora.init();
lora.sendData(data, sizeof(data));
```

### 数据处理Pipeline示例

```cpp
// 构建温度控制系统
AdcSource tempSensor;                 // 温度传感器
LowPassFilter filter(&tempSensor);    // 噪声滤波
LinearMapper mapper(&filter);         // 电压到温度转换
PidController<1> heaterCtrl(&mapper); // PID温度控制

// 配置PID参数
PidControllerConfig config;
config.Kp = 2.0f;
config.Ki = 0.5f; 
config.Kd = 0.1f;
config.setPoint = 25.0f; // 目标温度25°C
heaterCtrl.setConfig(config);

// 控制循环
while (true) {
    heaterCtrl.update();
    f32 heaterPower = heaterCtrl.getValue(0);
    pwm.setDutyCycle(heaterPower);
    wibot::os::sleep(100);
}
```

### 异步编程示例

```cpp
// 异步数据读取
AsyncSource source;
AsyncResult result = source.getResult();

// 启动异步操作
dma.startRead(buffer, size);

// 等待完成
Result status = result.wait(1000); // 等待最多1000ms
if (status.isOk()) {
    // 处理接收到的数据
} else if (status.isTimeout()) {
    // 处理超时
}
```

## AI助手操作指南

### 添加新功能时

1. 确定功能所属的层次（设备驱动、协议、HAL、基础组件等）
2. 在相应的目录下创建`.hpp`和`.cpp`文件
3. 遵循已有代码的命名和风格规范
4. 使用Doxygen注释格式编写文档
5. 在`CMakeLists.txt`中添加新文件到构建系统

### 修改现有代码时

1. 先阅读相关的头文件了解接口定义
2. 查看实现文件了解具体实现
3. 检查是否有相关的文档和示例
4. 保持修改与现有代码风格一致
5. 更新相关文档

### 调试问题时

1. 检查`docs/`目录下是否有相关文档
2. 查看`example/`目录下的示例代码
3. 使用RTT日志系统输出调试信息
4. 检查硬件配置是否正确（STM32CubeMX生成的代码）

### 添加新设备驱动时

1. 在`src/device/`下的相应子目录创建驱动代码
2. 实现设备初始化、读写等基本接口
3. 如需异步操作，集成AsyncSource/AsyncResult
4. 如需数据处理，考虑实现为Pipeline组件
5. 编写使用示例和文档

### 实现新协议时

1. 在`src/protocol/`下创建协议目录
2. 定义协议数据结构
3. 实现编码/解码功能
4. 提供易用的API接口
5. 编写单元测试

## 重要文件说明

- `src/base/type.hpp`: 统一的类型定义
- `src/base/buffer.hpp`: 缓冲区管理
- `src/base/circular-buffer.hpp`: 循环缓冲区
- `src/base/chrono.hpp`: 时间系统
- `src/os/os.hpp`: 操作系统抽象层统一接口
- `src/os/async.hpp`: 异步编程支持
- `src/hal/system.hpp`: 系统级功能（延时、时钟等）
- `src/pp/pipeline.hpp`: 数据流模型基础定义
- `src/pp/transform/`: 数据转换器组件（新增）
- `src/comm/protocol/`: 通信协议实现目录
- `src/comm/crc/`: CRC校验库
- `src/comm/modbus/`: Modbus协议实现
- `src/app/app-framework.hpp`: 应用程序框架
- `src/dsp/`: 数字信号处理库（新增）
- `src/math/`: 高级数学库（新增）

## 文档资源

详细文档位于`docs/`目录：

- `async-result-usage.md`: 异步编程指南
- `analog-input-README.md`: ADC使用指南
- `lowpass-filter-README.md`: 滤波器配置
- `AutoCalibration_Usage_Guide.md`: 自动校准系统
- `optimization_guide.md`: 优化指南
- 以及其他各种功能的详细文档

## 注意事项

1. **项目基础**: WibotLib设计为与STM32CubeMX生成的CMake项目配合使用
2. **RTOS配置**: 确保STM32CubeMX中正确配置了RTOS（如果使用）
3. **外设配置**: 硬件外设需要在STM32CubeMX中预先配置
4. **内存管理**: 库使用静态内存分配，注意堆栈大小配置
5. **中断安全**: 某些API不是中断安全的，需在文档中查看说明
6. **线程安全**: 多线程环境下注意使用互斥锁保护共享资源

## 许可证

本项目采用MIT许可证。

## 技术支持

- **GitHub Issues**: 报告bug和功能请求
- **文档**: 查看docs目录获取详细使用说明
- **示例代码**: 参考example目录中的完整示例

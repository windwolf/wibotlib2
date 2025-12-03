# Modbus 寄存器注册机制

## 概述

这是一个基于C++模板的Modbus寄存器管理系统，具有以下特点：

- **零运行时开销**: 使用编译期模板元编程，无运行时查找表
- **类型安全**: 编译期检查寄存器地址冲突、访问权限
- **内存高效**: 最小化内存占用，只存储实际数据
- **易于使用**: 通过类型系统访问寄存器，API简洁明了

## 核心组件

### 1. RegisterDef - 寄存器定义

定义单个寄存器的所有属性：

```cpp
template <u16 ADDR,                        // 寄存器地址
          RegisterType TYPE,                // 寄存器类型
          RegisterDataType DATA_TYPE,       // 数据类型
          RegisterAccess ACCESS,            // 访问权限
          u32 DEFAULT>                      // 默认值
struct RegisterDef;
```

**参数说明:**

- `ADDR`: 寄存器地址 (0-65535)
- `TYPE`: 寄存器类型
  - `RegisterType::kHoldingRegister` - 保持寄存器
  - `RegisterType::kInputRegister` - 输入寄存器
  - `RegisterType::kCoil` - 线圈
  - `RegisterType::kDiscreteInput` - 离散输入
- `DATA_TYPE`: 数据类型
  - `RegisterDataType::kUint16` - 16位无符号整数 (1个寄存器)
  - `RegisterDataType::kInt16` - 16位有符号整数 (1个寄存器)
  - `RegisterDataType::kUint32` - 32位无符号整数 (2个寄存器)
  - `RegisterDataType::kInt32` - 32位有符号整数 (2个寄存器)
  - `RegisterDataType::kFloat` - 32位浮点数 (2个寄存器)
- `ACCESS`: 访问权限
  - `RegisterAccess::kReadOnly` - 只读
  - `RegisterAccess::kWriteOnly` - 只写
  - `RegisterAccess::kReadWrite` - 读写
- `DEFAULT`: 默认值

### 2. RegisterMap - 寄存器映射

管理多个寄存器的集合：

```cpp
template <typename... RegDefs>
class RegisterMap;
```

### 3. DirectRegisterAccessor - 直接访问器

提供零开销的寄存器访问接口：

```cpp
template <typename RegMap>
class DirectRegisterAccessor;
```

## 使用示例

### 基本用法

```cpp
#include "protocol/modbus/register.hpp"

using namespace wibot::modbus;

// 1. 定义寄存器
using VoltageReg = RegisterDef<
    0x0000,                              // 地址: 0
    RegisterType::kHoldingRegister,      // 类型: 保持寄存器
    RegisterDataType::kUint16,           // 数据类型: u16
    RegisterAccess::kReadWrite,          // 权限: 读写
    3300>;                               // 默认值: 3300

using CurrentReg = RegisterDef<0x0001, 
                               RegisterType::kHoldingRegister,
                               RegisterDataType::kUint16,
                               RegisterAccess::kReadWrite,
                               1000>;

using TemperatureReg = RegisterDef<0x0002,
                                   RegisterType::kInputRegister,
                                   RegisterDataType::kInt16,
                                   RegisterAccess::kReadOnly,
                                   250>;

// 2. 创建寄存器映射
using MyRegisterMap = RegisterMap<VoltageReg, CurrentReg, TemperatureReg>;

// 3. 使用寄存器
void example() {
    MyRegisterMap registers;  // 自动初始化为默认值
    
    // 读写寄存器
    registers.set<VoltageReg>(3500);
    u16 voltage = registers.read<VoltageReg>();
    
    registers.set<CurrentReg>(1200);
    u16 current = registers.read<CurrentReg>();
    
    // 只读寄存器需要通过内部接口更新
    registers.get<TemperatureReg>().value() = 260;
    i16 temp = registers.read<TemperatureReg>();
}
```

### 32位数据类型

```cpp
// 32位数据类型占用2个连续寄存器
using PowerReg = RegisterDef<0x0003,
                             RegisterType::kHoldingRegister,
                             RegisterDataType::kUint32,
                             RegisterAccess::kReadWrite,
                             0>;

using ScaleReg = RegisterDef<0x0005,  // 注意：避开0x0004
                             RegisterType::kHoldingRegister,
                             RegisterDataType::kFloat,
                             RegisterAccess::kReadWrite,
                             0x3F800000>;  // 1.0的IEEE 754表示

void example32bit() {
    using MyMap = RegisterMap<PowerReg, ScaleReg>;
    MyMap regs;
    
    regs.set<PowerReg>(123456u);
    u32 power = regs.read<PowerReg>();
    
    regs.set<ScaleReg>(1.5f);
    f32 scale = regs.read<ScaleReg>();
}
```

### 缓冲区操作

```cpp
void bufferExample() {
    MyRegisterMap regs;
    u8 buffer[4];
    
    // 写入寄存器到缓冲区 (Modbus格式，大端序)
    regs.set<VoltageReg>(3300);
    regs.writeToBuffer<VoltageReg>(buffer);
    // buffer[0] = 0x0C, buffer[1] = 0xE4
    
    // 从缓冲区读取到寄存器
    buffer[0] = 0x04;
    buffer[1] = 0xB0;  // 1200
    regs.readFromBuffer<CurrentReg>(buffer);
    u16 current = regs.read<CurrentReg>();  // 1200
}
```

### 直接访问器

```cpp
void accessorExample() {
    MyRegisterMap regs;
    DirectRegisterAccessor<MyRegisterMap> accessor(regs);
    
    // 零开销访问
    accessor.write<VoltageReg>(3400);
    u16 voltage = accessor.read<VoltageReg>();
    
    // 获取寄存器引用
    auto& voltageReg = accessor.get<VoltageReg>();
    voltageReg.value() = 3500;
}
```

### 在设备类中使用

```cpp
class PowerMonitor {
public:
    PowerMonitor() {
        // 寄存器自动初始化为默认值
    }
    
    void setVoltage(u16 mv) {
        _registers.set<VoltageReg>(mv);
    }
    
    u16 getVoltage() const {
        return _registers.read<VoltageReg>();
    }
    
    void updateTemperature(i16 temp) {
        // 内部更新只读寄存器
        _registers.get<TemperatureReg>().value() = temp;
    }
    
    // 导出寄存器映射供Modbus通信使用
    MyRegisterMap& getRegisters() { return _registers; }

private:
    MyRegisterMap _registers;
};
```

## 编译期检查

系统提供以下编译期检查：

### 1. 访问权限检查

```cpp
// 编译错误: Register is read-only
// registers.set<TemperatureReg>(100);
```

### 2. 地址冲突检查

```cpp
using Reg1 = RegisterDef<0x0000>;
using Reg2 = RegisterDef<0x0000>;  // 地址重复

// 编译错误: Duplicate register address detected
// using BadMap = RegisterMap<Reg1, Reg2>;
```

### 3. 32位数据地址占用检查

```cpp
using Reg32 = RegisterDef<0x0003, 
                          RegisterType::kHoldingRegister,
                          RegisterDataType::kUint32>;  // 占用0x0003和0x0004

using Reg16 = RegisterDef<0x0004>;  // 冲突!

// 编译错误: Address conflict with 32-bit register
```

## 性能特性

### 零运行时开销

所有寄存器访问都在编译期确定，生成的代码等同于直接访问成员变量：

```cpp
// 这段代码:
registers.set<VoltageReg>(3300);
u16 v = registers.read<VoltageReg>();

// 编译后等同于:
voltageValue = 3300;
u16 v = voltageValue;
```

### 内存占用

- 每个u16寄存器: 2字节
- 每个u32/i32/float寄存器: 4字节
- 无额外的查找表或元数据开销

### 代码大小

- 模板代码在编译期展开，无运行时模板开销
- 未使用的寄存器代码会被编译器优化掉

## 最佳实践

1. **使用using定义寄存器类型**: 提高代码可读性和可维护性

```cpp
using VoltageReg = RegisterDef<0x0000, ...>;
using CurrentReg = RegisterDef<0x0001, ...>;
```

2. **集中定义寄存器**: 在单独的头文件中定义所有寄存器

```cpp
// device_registers.hpp
namespace MyDevice {
    using VoltageReg = ...;
    using CurrentReg = ...;
    using MyRegisterMap = RegisterMap<VoltageReg, CurrentReg, ...>;
}
```

3. **只读寄存器的更新**: 使用value()方法直接访问

```cpp
// 好的做法
void updateSensorData(i16 temp) {
    _registers.get<TemperatureReg>().value() = temp;
}

// 不要这样做 (编译错误)
// _registers.set<TemperatureReg>(temp);
```

4. **32位数据的地址规划**: 预留连续地址空间

```cpp
// 好的做法
using Reg16_1 = RegisterDef<0x0000>;  // 地址0
using Reg32_1 = RegisterDef<0x0001>;  // 地址1-2
using Reg16_2 = RegisterDef<0x0003>;  // 地址3

// 不好的做法 (会导致地址冲突)
// using Reg32_1 = RegisterDef<0x0001>;  // 地址1-2
// using Reg16_2 = RegisterDef<0x0002>;  // 冲突!
```

## 与Modbus通信集成

```cpp
class ModbusDevice {
public:
    bool handleReadHoldingRegisters(u16 addr, u16 count, u8* response) {
        if (addr == VoltageReg::Address && count == 1) {
            _registers.writeToBuffer<VoltageReg>(response);
            return true;
        }
        if (addr == CurrentReg::Address && count == 1) {
            _registers.writeToBuffer<CurrentReg>(response);
            return true;
        }
        return false;  // 地址不存在
    }
    
    bool handleWriteHoldingRegisters(u16 addr, u16 count, const u8* data) {
        if (addr == VoltageReg::Address && count == 1) {
            _registers.readFromBuffer<VoltageReg>(data);
            return true;
        }
        if (addr == CurrentReg::Address && count == 1) {
            _registers.readFromBuffer<CurrentReg>(data);
            return true;
        }
        return false;
    }

private:
    MyRegisterMap _registers;
};
```

## 常见问题

### Q: 如何处理运行时地址查找？

A: 对于需要运行时地址查找的场景，可以使用switch-case或if-else链：

```cpp
bool readRegisterByAddress(u16 addr, u8* buffer) {
    switch(addr) {
        case VoltageReg::Address:
            registers.writeToBuffer<VoltageReg>(buffer);
            return true;
        case CurrentReg::Address:
            registers.writeToBuffer<CurrentReg>(buffer);
            return true;
        default:
            return false;
    }
}
```

编译器会将其优化为跳转表，性能接近直接访问。

### Q: 如何支持更多数据类型？

A: 扩展RegisterDataType枚举和RegisterItem的条件编译逻辑即可。

### Q: 寄存器数量有限制吗？

A: 理论上无限制，但过多的寄存器会增加编译时间。建议单个RegisterMap不超过100个寄存器。

## 文件结构

- `register.hpp` - 核心寄存器定义和映射
- `register_handler.hpp` - Modbus通信处理器
- `register_example.hpp` - 使用示例
- `README.md` - 本文档

## 许可证

本代码属于wibotlib库的一部分。

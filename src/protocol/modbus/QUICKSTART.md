# 快速开始指南

## 3分钟上手Modbus寄存器系统

### 步骤1: 定义你的寄存器

```cpp
#include "protocol/modbus/register.hpp"

using namespace wibot::modbus;

// 定义一个电压寄存器
using VoltageReg = RegisterDef<
    0x0000,                              // 地址
    RegisterType::kHoldingRegister,      // 类型
    RegisterDataType::kUint16,           // 数据类型
    RegisterAccess::kReadWrite,          // 权限
    3300>;                               // 默认值 3.3V
```

### 步骤2: 创建寄存器映射

```cpp
// 定义多个寄存器
using CurrentReg = RegisterDef<0x0001, RegisterType::kHoldingRegister,
                               RegisterDataType::kUint16, RegisterAccess::kReadWrite, 1000>;

using TempReg = RegisterDef<0x0002, RegisterType::kInputRegister,
                            RegisterDataType::kInt16, RegisterAccess::kReadOnly, 250>;

// 创建映射
using MyRegisterMap = RegisterMap<VoltageReg, CurrentReg, TempReg>;
```

### 步骤3: 使用寄存器

```cpp
class MyDevice {
public:
    void setVoltage(u16 mv) {
        regs.set<VoltageReg>(mv);
    }
    
    u16 getVoltage() const {
        return regs.read<VoltageReg>();
    }
    
    void updateTemp(i16 temp) {
        regs.get<TempReg>().value() = temp;  // 只读寄存器用value()
    }
    
private:
    MyRegisterMap regs;
};
```

## 关键特性

### ✓ 编译期类型安全

```cpp
// ✗ 编译错误: 只读寄存器不能写
// regs.set<TempReg>(100);

// ✗ 编译错误: 地址冲突
// using Dup = RegisterDef<0x0000>;
// using BadMap = RegisterMap<VoltageReg, Dup>;
```

### ✓ 零运行时开销

```cpp
regs.set<VoltageReg>(3300);
// 编译后等同于: voltageValue = 3300;
```

### ✓ 支持多种数据类型

```cpp
// 16位整数 (1个寄存器)
RegisterDataType::kUint16
RegisterDataType::kInt16

// 32位数据 (2个寄存器)
RegisterDataType::kUint32
RegisterDataType::kInt32
RegisterDataType::kFloat
```

## 常用操作

### 读写寄存器

```cpp
MyRegisterMap regs;

// 写入
regs.set<VoltageReg>(5000);

// 读取
u16 voltage = regs.read<VoltageReg>();
```

### 缓冲区操作 (Modbus通信)

```cpp
u8 buffer[4];

// 写入缓冲区 (大端序)
regs.writeToBuffer<VoltageReg>(buffer);

// 从缓冲区读取
regs.readFromBuffer<VoltageReg>(buffer);
```

### 32位数据

```cpp
using PowerReg = RegisterDef<0x0003,
                             RegisterType::kHoldingRegister,
                             RegisterDataType::kUint32,
                             RegisterAccess::kReadWrite,
                             0>;

// 注意: 占用0x0003和0x0004两个地址
// 下一个寄存器应该从0x0005开始

regs.set<PowerReg>(123456u);
u32 power = regs.read<PowerReg>();
```

## 完整示例

查看以下文件了解更多：

- `register_example.hpp` - 基础示例
- `register_test.hpp` - 完整的电源监控设备示例
- `README.md` - 详细文档

## 下一步

1. 查看 `register_test.hpp` 中的 `PowerMonitorController` 类
2. 了解如何集成到Modbus通信
3. 学习编译期检查机制

---

**性能提示**: 所有寄存器访问都在编译期解析，无运行时查找！

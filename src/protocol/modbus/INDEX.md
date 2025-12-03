# Modbus寄存器系统 - 文件索引

## 核心实现文件

### 1. `register.hpp` ⭐
**核心寄存器实现**

包含以下组件：
- `RegisterType` - 寄存器类型枚举
- `RegisterAccess` - 访问权限枚举
- `RegisterDataType` - 数据类型枚举
- `RegisterDef<>` - 寄存器定义模板
- `RegisterItem<>` - 寄存器数据存储类
- `RegisterMap<>` - 寄存器映射容器

**用途**: 这是系统的核心，定义了所有基础类型和模板。

### 2. `register_handler.hpp`
**Modbus通信辅助类**

包含以下组件：
- `ModbusSlaveRegisterHandler<>` - Modbus从机寄存器处理
- `DirectRegisterAccessor<>` - 直接寄存器访问器
- 辅助宏定义

**用途**: 简化Modbus通信中的寄存器读写操作。

## 示例和文档

### 3. `register_example.hpp`
**基础使用示例**

展示内容：
- 定义各种类型的寄存器
- 创建寄存器映射
- 基本的读写操作
- 编译期检查示例

**用途**: 快速了解如何定义和使用寄存器。

### 4. `register_test.hpp`
**完整应用示例**

包含：
- 完整的电源监控设备实现
- 12个不同类型的寄存器定义
- Modbus通信处理完整实现
- 实际应用场景演示

**用途**: 学习如何在实际项目中使用该系统。

### 5. `README.md`
**详细文档**

内容：
- 系统概述
- 核心组件说明
- 详细的使用示例
- 编译期检查说明
- 性能特性
- 最佳实践
- 常见问题解答

**用途**: 完整的参考文档。

### 6. `QUICKSTART.md`
**快速入门指南**

内容：
- 3分钟上手教程
- 3个简单步骤
- 关键特性概览
- 常用操作示例

**用途**: 新手快速上手。

### 7. `IMPLEMENTATION.md`
**实现总结文档**

内容：
- 核心设计思想
- 性能特性对比
- 关键技术点
- 内存占用分析
- 性能测试数据
- 扩展性说明

**用途**: 深入理解实现原理和性能优势。

## 文件关系图

```
register.hpp (核心)
    ├── register_example.hpp (基础示例)
    ├── register_test.hpp (完整示例)
    └── register_handler.hpp (通信辅助)

README.md (详细文档)
    ├── QUICKSTART.md (快速入门)
    └── IMPLEMENTATION.md (实现细节)
```

## 推荐阅读顺序

### 如果你是新手：
1. `QUICKSTART.md` - 快速了解基本用法
2. `register_example.hpp` - 查看基础示例代码
3. `register_test.hpp` - 学习完整应用
4. `README.md` - 深入了解所有特性

### 如果你想了解原理：
1. `IMPLEMENTATION.md` - 理解设计思想
2. `register.hpp` - 查看核心实现
3. `README.md` - 了解所有特性

### 如果你想立即使用：
1. `QUICKSTART.md` - 3分钟上手
2. 复制 `register_example.hpp` 中的代码模板
3. 根据需求修改寄存器定义
4. 开始使用！

## 核心特性总结

✅ **零运行时开销** - 所有访问在编译期确定  
✅ **最小内存占用** - 仅存储实际数据  
✅ **类型安全** - 编译期检查所有错误  
✅ **易于使用** - 简洁的API  
✅ **高性能** - 单周期寄存器访问  
✅ **可扩展** - 支持自定义数据类型  

## 支持的寄存器类型

- **保持寄存器** (Holding Register) - 可读写
- **输入寄存器** (Input Register) - 只读
- **线圈** (Coil) - 可读写
- **离散输入** (Discrete Input) - 只读

## 支持的数据类型

- `u16` / `i16` - 16位整数 (占用1个寄存器)
- `u32` / `i32` - 32位整数 (占用2个寄存器)
- `float` - 32位浮点数 (占用2个寄存器)

## 快速示例

```cpp
#include "register.hpp"

using namespace wibot::modbus;

// 定义寄存器
using VoltageReg = RegisterDef<0x0000, RegisterType::kHoldingRegister,
                               RegisterDataType::kUint16, RegisterAccess::kReadWrite, 3300>;

// 创建映射
using MyMap = RegisterMap<VoltageReg>;

// 使用
MyMap regs;
regs.set<VoltageReg>(5000);
u16 v = regs.read<VoltageReg>();
```

## 获取帮助

- 查看 `README.md` 的常见问题部分
- 参考 `register_test.hpp` 中的完整示例
- 阅读代码注释

---

**创建日期**: 2025-11-28  
**版本**: 1.0  
**作者**: GitHub Copilot

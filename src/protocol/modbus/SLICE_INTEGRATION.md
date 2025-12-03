# Modbus寄存器系统 + Slice集成

## 集成优势

通过集成现有的`Buffer`和`Slice`基础设施，寄存器系统获得了以下增强功能：

✅ **字节序支持** - 自动处理大端/小端转换  
✅ **类型安全** - 利用Slice的类型化访问方法  
✅ **代码复用** - 复用经过验证的缓冲区操作  
✅ **统一API** - 与项目其他部分保持一致  

## 核心改进

### 1. 替换自定义缓冲区函数

**之前**:
```cpp
// 自定义序列化函数
namespace detail {
    inline void writeU16ToBuffer(u16 value, u8* buffer) {
        buffer[0] = (value >> 8) & 0xFF;
        buffer[1] = value & 0xFF;
    }
}
```

**现在**:
```cpp
// 使用现有Slice API
void writeToSlice(Slice& slice, u16 offset = 0) const {
    if constexpr (RegDef::DataType == RegisterDataType::kUint16) {
        slice.setUint16(offset, _value, Endian::kBig);  // 支持字节序!
    }
}
```

### 2. 新增Slice支持方法

```cpp
class RegisterItem {
    // 原有方法
    void writeToBuffer(u8* buffer) const;
    void readFromBuffer(const u8* buffer);
    
    // 新增Slice方法
    void writeToSlice(Slice& slice, u16 offset = 0) const;
    void readFromSlice(const Slice& slice, u16 offset = 0);
    static constexpr u16 getBufferSize();
};

class RegisterMap {
    // 单个寄存器Slice操作
    template<typename RegDef>
    void writeToSlice(Slice& slice, u16 offset = 0) const;
    
    template<typename RegDef>  
    void readFromSlice(const Slice& slice, u16 offset = 0);
    
    // 批量寄存器Slice操作
    template<typename... SelectedRegDefs>
    void writeRegistersToSlice(Slice& slice) const;
    
    template<typename... SelectedRegDefs>
    void readRegistersFromSlice(const Slice& slice);
};
```

## 使用场景

### 1. 基础寄存器操作

```cpp
using VoltageReg = RegisterDef<0x0000, RegisterType::kHoldingRegister, RegisterDataType::kUint16>;
using MyMap = RegisterMap<VoltageReg>;

MyMap regs;
regs.set<VoltageReg>(5000);

// 写入到Slice
Buffer<4> buffer;
Slice slice = buffer;
regs.writeToSlice<VoltageReg>(slice, 0);
// slice包含: [0x13, 0x88] (大端序)
```

### 2. 字节序控制

```cpp
// Modbus标准 (大端序)
slice.setUint16(0, voltage, Endian::kBig);

// 某些设备使用小端序
slice.setUint16(0, voltage, Endian::kLittle);
```

### 3. 批量操作

```cpp
// 批量写入多个寄存器
regs.writeRegistersToSlice<VoltageReg, CurrentReg, TemperatureReg>(slice);
// 自动按顺序写入，计算偏移量

// 批量读取
regs.readRegistersFromSlice<VoltageReg, CurrentReg, TemperatureReg>(slice);
```

### 4. Modbus通信集成

```cpp
bool handleReadHoldingRegisters(u16 addr, u16 count, Buffer<256>& response) {
    if (addr == VoltageReg::Address && count == 1) {
        Slice responseSlice = response;  // 自动转换
        regs.writeToSlice<VoltageReg>(responseSlice, 0);
        response.size = 2;
        return true;
    }
    return false;
}
```

### 5. 数据导出/导入

```cpp
void exportAllData(Buffer<32>& exportBuffer) {
    Slice slice = exportBuffer;
    u16 offset = 0;
    
    // 按地址顺序导出
    regs.writeToSlice<VoltageReg>(slice, offset); offset += 2;
    regs.writeToSlice<CurrentReg>(slice, offset); offset += 2; 
    regs.writeToSlice<PowerReg>(slice, offset);   offset += 4;  // 32位
    
    exportBuffer.size = offset;
}
```

## API参考

### RegisterItem 新增方法

| 方法                           | 说明                |
| ------------------------------ | ------------------- |
| `writeToSlice(slice, offset)`  | 写入到Slice指定偏移 |
| `readFromSlice(slice, offset)` | 从Slice指定偏移读取 |
| `getBufferSize()`              | 获取所需缓冲区大小  |

### RegisterMap 新增方法  

| 方法                                        | 说明                  |
| ------------------------------------------- | --------------------- |
| `writeToSlice<RegDef>(slice, offset)`       | 写入单个寄存器到Slice |
| `readFromSlice<RegDef>(slice, offset)`      | 从Slice读取单个寄存器 |
| `writeRegistersToSlice<RegDefs...>(slice)`  | 批量写入多个寄存器    |
| `readRegistersFromSlice<RegDefs...>(slice)` | 批量读取多个寄存器    |

## 性能特点

### 零拷贝操作
```cpp
Buffer<256> modbusBuffer;
Slice slice = modbusBuffer;  // 无拷贝，只是包装

// 直接操作底层内存
regs.writeToSlice<VoltageReg>(slice, 0);  // 零拷贝写入
```

### 编译期优化
- Slice方法调用被完全内联
- 字节序转换在编译期确定
- 无运行时类型检查开销

### 类型安全
```cpp
// 编译期检查数据类型匹配
slice.setUint16(0, u16Value);    // ✅ 类型匹配
slice.setUint16(0, u32Value);    // ❌ 编译错误
```

## 向后兼容

所有原有API保持不变：
```cpp
// 原有方法仍然可用
regs.set<VoltageReg>(5000);
u16 v = regs.read<VoltageReg>();

u8 buffer[4];
regs.writeToBuffer<VoltageReg>(buffer);
regs.readFromBuffer<VoltageReg>(buffer);
```

## 最佳实践

### 1. 优先使用Slice方法
```cpp
// 推荐：使用Slice
Buffer<8> buffer;
Slice slice = buffer;
regs.writeToSlice<VoltageReg>(slice, 0);

// 不推荐：直接使用原始指针
u8* rawPtr = buffer.data;
regs.writeToBuffer<VoltageReg>(rawPtr);
```

### 2. 利用批量操作
```cpp
// 高效：单次批量操作
regs.writeRegistersToSlice<VoltageReg, CurrentReg, TempReg>(slice);

// 低效：多次单独操作  
regs.writeToSlice<VoltageReg>(slice, 0);
regs.writeToSlice<CurrentReg>(slice, 2);
regs.writeToSlice<TempReg>(slice, 4);
```

### 3. 合理设置缓冲区大小
```cpp
// 计算所需缓冲区大小
constexpr u16 totalSize = 
    VoltageReg::RegisterCount * 2 +    // 2字节
    CurrentReg::RegisterCount * 2 +    // 2字节
    PowerReg::RegisterCount * 2;       // 4字节 (32位)

Buffer<totalSize> buffer;  // 精确大小
```

## 总结

集成Slice基础设施后，Modbus寄存器系统获得了：

- **更强的功能** - 字节序支持、类型安全
- **更好的性能** - 零拷贝、编译期优化  
- **更简洁的代码** - 复用现有基础设施
- **更好的一致性** - 与项目其他部分API统一

这是一个完美的**组合优化**例子！🎉
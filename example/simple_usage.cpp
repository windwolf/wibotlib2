/**
 * @file simple_usage.cpp
 * @brief 最简单的Modbus寄存器使用示例
 * 
 * 这个文件演示了最基本的使用方法，适合快速验证和学习
 */

#include "register.hpp"
#include <stdio.h>

using namespace wibot::modbus;

// ============================================================================
// 步骤1: 定义你的寄存器
// ============================================================================

// 电压寄存器: 地址0x0000, u16类型, 可读写, 默认3300mV
using VoltageReg = RegisterDef<0x0000, RegisterType::kHoldingRegister, RegisterDataType::kUint16,
                               RegisterAccess::kReadWrite, 3300>;

// 电流寄存器: 地址0x0001, u16类型, 可读写, 默认1000mA
using CurrentReg = RegisterDef<0x0001, RegisterType::kHoldingRegister, RegisterDataType::kUint16,
                               RegisterAccess::kReadWrite, 1000>;

// 温度寄存器: 地址0x0002, i16类型, 只读, 默认250 (25.0℃)
using TempReg = RegisterDef<0x0002, RegisterType::kInputRegister, RegisterDataType::kInt16,
                            RegisterAccess::kReadOnly, 250>;

// ============================================================================
// 步骤2: 创建寄存器映射
// ============================================================================

using MyRegisterMap = RegisterMap<VoltageReg, CurrentReg, TempReg>;

// ============================================================================
// 步骤3: 使用寄存器
// ============================================================================

void basicUsageExample() {
    printf("=== Modbus寄存器基本使用示例 ===\n\n");

    // 创建寄存器映射实例（自动初始化为默认值）
    MyRegisterMap registers;

    // 1. 读取默认值
    printf("1. 读取默认值:\n");
    printf("   电压: %u mV\n", registers.read<VoltageReg>());
    printf("   电流: %u mA\n", registers.read<CurrentReg>());
    printf("   温度: %d (%.1f℃)\n\n", registers.read<TempReg>(), registers.read<TempReg>() / 10.0);

    // 2. 写入新值
    printf("2. 设置新值:\n");
    registers.set<VoltageReg>(5000);  // 设置为5000mV (5V)
    registers.set<CurrentReg>(2000);  // 设置为2000mA (2A)
    printf("   设置电压: 5000 mV\n");
    printf("   设置电流: 2000 mA\n\n");

    // 3. 读取更新后的值
    printf("3. 读取更新后的值:\n");
    printf("   电压: %u mV\n", registers.read<VoltageReg>());
    printf("   电流: %u mA\n\n", registers.read<CurrentReg>());

    // 4. 更新只读寄存器（通过内部接口）
    printf("4. 更新温度传感器读数:\n");
    registers.get<TempReg>().value() = 320;  // 32.0℃
    printf("   温度: %d (%.1f℃)\n\n", registers.read<TempReg>(), registers.read<TempReg>() / 10.0);

    // 5. Modbus缓冲区操作
    printf("5. Modbus缓冲区操作:\n");
    u8 buffer[4];

    // 将电压值写入缓冲区（Modbus大端序格式）
    registers.writeToBuffer<VoltageReg>(buffer);
    printf("   电压缓冲区: 0x%02X 0x%02X\n", buffer[0], buffer[1]);

    // 从缓冲区读取电流值
    buffer[0] = 0x04;  // 高字节
    buffer[1] = 0xB0;  // 低字节 (1200)
    registers.readFromBuffer<CurrentReg>(buffer);
    printf("   从缓冲区读取电流: %u mA\n\n", registers.read<CurrentReg>());

    printf("=== 示例完成 ===\n");
}

// ============================================================================
// 32位数据类型示例
// ============================================================================

using PowerReg = RegisterDef<0x0010, RegisterType::kHoldingRegister, RegisterDataType::kUint32,
                             RegisterAccess::kReadWrite, 0>;

using ScaleReg = RegisterDef<0x0012,  // 注意：跳过0x0011
                             RegisterType::kHoldingRegister, RegisterDataType::kFloat,
                             RegisterAccess::kReadWrite,
                             0x3F800000>;  // 1.0

using ExtendedMap = RegisterMap<PowerReg, ScaleReg>;

void extendedDataTypeExample() {
    printf("\n=== 32位数据类型示例 ===\n\n");

    ExtendedMap registers;

    // 1. 32位无符号整数
    printf("1. 功率寄存器 (u32):\n");
    registers.set<PowerReg>(123456u);
    printf("   设置功率: %lu mW\n", (unsigned long)registers.read<PowerReg>());

    u8 buffer[4];
    registers.writeToBuffer<PowerReg>(buffer);
    printf("   缓冲区: 0x%02X 0x%02X 0x%02X 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3]);
    printf("\n");

    // 2. 浮点数
    printf("2. 比例系数寄存器 (float):\n");
    registers.set<ScaleReg>(1.5f);
    printf("   设置系数: %.2f\n", registers.read<ScaleReg>());

    registers.writeToBuffer<ScaleReg>(buffer);
    printf("   缓冲区: 0x%02X 0x%02X 0x%02X 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3]);

    printf("\n=== 示例完成 ===\n");
}

// ============================================================================
// 编译期检查示例（这些代码会导致编译错误）
// ============================================================================

/*
void compileTimeCheckExamples() {
    MyRegisterMap registers;
    
    // 错误1: 尝试写入只读寄存器
    // registers.set<TempReg>(100);  
    // 编译错误: Register is read-only
    
    // 错误2: 地址冲突
    // using DupReg = RegisterDef<0x0000>;
    // using BadMap = RegisterMap<VoltageReg, DupReg>;
    // 编译错误: Duplicate register address detected
}
*/

// ============================================================================
// 主函数
// ============================================================================

#if 0  // 设置为1来编译这个示例
int main() {
    basicUsageExample();
    extendedDataTypeExample();
    return 0;
}
#endif

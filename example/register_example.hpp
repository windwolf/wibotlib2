#pragma once

#include "register.hpp"

/**
 * @file register_example.hpp
 * @brief Modbus寄存器使用示例
 * 
 * 这个文件展示如何使用寄存器注册机制定义和访问Modbus寄存器
 */

namespace wibot::modbus::example {

// ============================================================================
// 1. 定义寄存器
// ============================================================================

// 电压寄存器 - 地址0x0000, 保持寄存器, u16类型, 可读写, 默认值3300
using VoltageReg = RegisterDef<0x0000, RegisterType::kHoldingRegister, RegisterDataType::kUint16,
                               RegisterAccess::kReadWrite, 3300>;

// 电流寄存器 - 地址0x0001, 保持寄存器, u16类型, 可读写, 默认值1000
using CurrentReg = RegisterDef<0x0001, RegisterType::kHoldingRegister, RegisterDataType::kUint16,
                               RegisterAccess::kReadWrite, 1000>;

// 温度寄存器 - 地址0x0002, 输入寄存器, i16类型, 只读, 默认值250 (25.0℃)
using TemperatureReg = RegisterDef<0x0002, RegisterType::kInputRegister, RegisterDataType::kInt16,
                                   RegisterAccess::kReadOnly, 250>;

// 功率寄存器 - 地址0x0003, 保持寄存器, u32类型, 只读, 默认值0
// 注意: u32类型会占用2个连续的寄存器地址 (0x0003和0x0004)
using PowerReg = RegisterDef<0x0003, RegisterType::kHoldingRegister, RegisterDataType::kUint32,
                             RegisterAccess::kReadOnly, 0>;

// 比例系数寄存器 - 地址0x0005, 保持寄存器, float类型, 可读写, 默认值1.0
using ScaleFactorReg = RegisterDef<0x0005, RegisterType::kHoldingRegister, RegisterDataType::kFloat,
                                   RegisterAccess::kReadWrite,
                                   0x3F800000>;  // 1.0的IEEE 754表示

// 设备状态寄存器 - 地址0x0007, 保持寄存器, u16类型, 可读写, 默认值0
using DeviceStatusReg = RegisterDef<0x0007, RegisterType::kHoldingRegister,
                                    RegisterDataType::kUint16, RegisterAccess::kReadWrite, 0>;

// ============================================================================
// 2. 创建寄存器映射
// ============================================================================

// 将所有寄存器定义传递给RegisterMap
using MyRegisterMap =
    RegisterMap<VoltageReg, CurrentReg, TemperatureReg, PowerReg, ScaleFactorReg, DeviceStatusReg>;

// ============================================================================
// 3. 使用示例
// ============================================================================

class DeviceController {
   public:
    DeviceController() {
        // 寄存器会自动初始化为默认值
    }

    // 设置电压
    void setVoltage(u16 voltage) {
        registers.set<VoltageReg>(voltage);
    }

    // 读取电压
    u16 getVoltage() const {
        return registers.read<VoltageReg>();
    }

    // 设置电流
    void setCurrent(u16 current) {
        registers.set<CurrentReg>(current);
    }

    // 读取电流
    u16 getCurrent() const {
        return registers.read<CurrentReg>();
    }

    // 更新温度 (只读寄存器，通过内部value()方法直接访问)
    void updateTemperature(i16 temp) {
        registers.get<TemperatureReg>().value() = temp;
    }

    // 读取温度
    i16 getTemperature() const {
        return registers.read<TemperatureReg>();
    }

    // 更新功率 (u32类型)
    void updatePower(u32 power) {
        registers.get<PowerReg>().value() = power;
    }

    // 读取功率
    u32 getPower() const {
        return registers.read<PowerReg>();
    }

    // 设置比例系数 (float类型)
    void setScaleFactor(f32 factor) {
        registers.set<ScaleFactorReg>(factor);
    }

    // 读取比例系数
    f32 getScaleFactor() const {
        return registers.read<ScaleFactorReg>();
    }

    // 将电压寄存器数据写入缓冲区
    void writeVoltageToBuffer(u8* buffer) const {
        registers.writeToBuffer<VoltageReg>(buffer);
    }

    // 从缓冲区更新电流寄存器
    void updateCurrentFromBuffer(const u8* buffer) {
        registers.readFromBuffer<CurrentReg>(buffer);
    }

    // 获取寄存器映射的引用（用于Modbus通信）
    MyRegisterMap& getRegisterMap() {
        return registers;
    }

    const MyRegisterMap& getRegisterMap() const {
        return registers;
    }

   private:
    MyRegisterMap registers;
};

// ============================================================================
// 4. 编译期检查示例
// ============================================================================

// 以下代码会在编译期报错，因为温度寄存器是只读的
// void testCompileError() {
//     MyRegisterMap regs;
//     regs.set<TemperatureReg>(100);  // 编译错误: Register is read-only
// }

// 以下代码会在编译期报错，因为地址重复
// using DuplicateReg = RegisterDef<0x0000>;  // 与VoltageReg地址重复
// using BadMap = RegisterMap<VoltageReg, DuplicateReg>;  // 编译错误: Duplicate register address

}  // namespace wibot::modbus::example

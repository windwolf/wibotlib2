#pragma once

#include "register.hpp"

/**
 * @file optimized_example.hpp  
 * @brief 优化后的Modbus寄存器使用示例
 * 
 * 展示使用标准库模板元编程实现的简洁版本
 */

namespace wibot::modbus::optimized_example {

// ============================================================================
// 寄存器定义 - 使用简洁的语法
// ============================================================================

// 简单的16位寄存器
using VoltageReg = RegisterDef<0x0000, RegisterType::kHoldingRegister, RegisterDataType::kUint16,
                               RegisterAccess::kReadWrite, 3300>;
using CurrentReg = RegisterDef<0x0001, RegisterType::kHoldingRegister, RegisterDataType::kUint16,
                               RegisterAccess::kReadWrite, 1000>;

// 只读的温度寄存器
using TemperatureReg = RegisterDef<0x0002, RegisterType::kInputRegister, RegisterDataType::kInt16,
                                   RegisterAccess::kReadOnly, 250>;

// 32位功率寄存器 (占用2个寄存器地址)
using PowerReg = RegisterDef<0x0003, RegisterType::kInputRegister, RegisterDataType::kUint32,
                             RegisterAccess::kReadOnly, 0>;

// 浮点校准系数 (占用2个寄存器地址)
using CalibrationReg = RegisterDef<0x0005, RegisterType::kHoldingRegister, RegisterDataType::kFloat,
                                   RegisterAccess::kReadWrite, 0x3F800000>;  // 1.0f

// 状态寄存器
using StatusReg = RegisterDef<0x0007, RegisterType::kHoldingRegister, RegisterDataType::kUint16,
                              RegisterAccess::kReadOnly, 0>;

// ============================================================================
// 寄存器映射
// ============================================================================

using DeviceRegisterMap =
    RegisterMap<VoltageReg, CurrentReg, TemperatureReg, PowerReg, CalibrationReg, StatusReg>;

// ============================================================================
// 设备控制器 - 展示零开销访问
// ============================================================================

class OptimizedDevice {
   public:
    OptimizedDevice() {
        // 寄存器自动初始化为默认值
        // 编译期检查确保无地址冲突
    }

    // ========================================================================
    // 零开销的直接访问
    // ========================================================================

    void setVoltage(u16 mv) {
        // 编译期展开为: _registers.get<VoltageReg>().set(mv)
        // 进一步优化为: voltageValue = mv
        registers_.set<VoltageReg>(mv);
    }

    u16 getVoltage() const {
        // 编译期展开为直接访问，零开销
        return registers_.read<VoltageReg>();
    }

    void setCurrent(u16 ma) {
        registers_.set<CurrentReg>(ma);
    }

    u16 getCurrent() const {
        return registers_.read<CurrentReg>();
    }

    // 只读寄存器的内部更新
    void updateTemperature(i16 temp) {
        registers_.get<TemperatureReg>().value() = temp;
    }

    i16 getTemperature() const {
        return registers_.read<TemperatureReg>();
    }

    // 32位数据操作
    void updatePower(u32 power) {
        registers_.get<PowerReg>().value() = power;
    }

    u32 getPower() const {
        return registers_.read<PowerReg>();
    }

    // 浮点数操作
    void setCalibration(f32 factor) {
        registers_.set<CalibrationReg>(factor);
    }

    f32 getCalibration() const {
        return registers_.read<CalibrationReg>();
    }

    // ========================================================================
    // Modbus缓冲区操作 - 编译期优化
    // ========================================================================

    void writeVoltageToBuffer(u8* buffer) const {
        // 编译期确定寄存器类型，直接调用对应方法
        registers_.writeToBuffer<VoltageReg>(buffer);
    }

    void updateCurrentFromBuffer(const u8* buffer) {
        registers_.readFromBuffer<CurrentReg>(buffer);
    }

    void writePowerToBuffer(u8* buffer) const {
        // 32位数据，自动写入4字节
        registers_.writeToBuffer<PowerReg>(buffer);
    }

    void writeCalibrationToBuffer(u8* buffer) const {
        // 浮点数，自动转换为IEEE 754格式
        registers_.writeToBuffer<CalibrationReg>(buffer);
    }

    // ========================================================================
    // 批量操作示例
    // ========================================================================

    void updateSensorData(u16 voltage, u16 current, i16 temp) {
        // 这些操作都是零开销的直接赋值
        registers_.get<VoltageReg>().value()     = voltage;
        registers_.get<CurrentReg>().value()     = current;
        registers_.get<TemperatureReg>().value() = temp;

        // 计算功率 (电压 * 电流 / 1000)
        u32 power                          = (static_cast<u32>(voltage) * current) / 1000;
        registers_.get<PowerReg>().value() = power;
    }

    // 获取所有测量值
    struct MeasurementData {
        u16 voltage;
        u16 current;
        i16 temperature;
        u32 power;
        f32 calibration;
    };

    MeasurementData getAllMeasurements() const {
        return {registers_.read<VoltageReg>(), registers_.read<CurrentReg>(),
                registers_.read<TemperatureReg>(), registers_.read<PowerReg>(),
                registers_.read<CalibrationReg>()};
    }

    // 导出寄存器映射供外部使用
    DeviceRegisterMap& getRegisters() {
        return registers_;
    }
    const DeviceRegisterMap& getRegisters() const {
        return registers_;
    }

   private:
    DeviceRegisterMap registers_;
};

// ============================================================================
// 使用示例
// ============================================================================

inline void runOptimizedExample() {
    OptimizedDevice device;

    // 1. 基本操作 - 零开销
    device.setVoltage(5000);  // 5V
    device.setCurrent(2000);  // 2A
    device.setCalibration(1.05f);

    // 2. 读取值 - 零开销
    u16 voltage = device.getVoltage();
    u16 current = device.getCurrent();
    f32 calib   = device.getCalibration();
    (void)voltage;
    (void)current;
    (void)calib;  // 避免未使用警告

    // 3. 更新传感器数据
    device.updateSensorData(4980, 1950, 320);  // 4.98V, 1.95A, 32.0°C

    // 4. 获取所有数据
    auto measurements = device.getAllMeasurements();
    (void)measurements;

    // 5. Modbus缓冲区操作
    u8 buffer[8];

    // 写入电压到缓冲区 (大端序)
    device.writeVoltageToBuffer(buffer);
    // buffer[0] = 0x13, buffer[1] = 0x78 (5000)

    // 写入功率到缓冲区 (32位，4字节)
    device.writePowerToBuffer(buffer);

    // 写入校准系数到缓冲区 (IEEE 754格式)
    device.writeCalibrationToBuffer(buffer);

    // 6. 编译期检查演示
    // 以下代码会产生编译错误:

    // device.setTemperature(100); // 编译错误: TemperatureReg是只读的

    // using ConflictReg = RegisterDef<0x0000>; // 编译错误: 地址与VoltageReg冲突
    // using BadMap = RegisterMap<VoltageReg, ConflictReg>;
}

// ============================================================================
// 性能对比示例
// ============================================================================

/**
 * @brief 传统实现 vs 本实现的性能对比
 */
inline void performanceComparison() {
    OptimizedDevice device;

    // 传统map实现 (伪代码):
    // std::map<u16, u16> registers;
    // registers[0x0000] = 5000;  // 需要哈希查找，约50-100个CPU周期

    // 本实现:
    device.setVoltage(5000);  // 直接赋值，1个CPU周期!

    // 编译后的汇编大致等价于:
    // MOV [device_voltage_offset], 5000
}

}  // namespace wibot::modbus::optimized_example
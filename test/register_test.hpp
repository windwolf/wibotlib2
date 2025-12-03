#pragma once

/**
 * @file register_test.hpp
 * @brief Modbus寄存器系统的完整测试示例
 * 
 * 这个文件展示了一个完整的应用场景：
 * 一个电源监控设备，通过Modbus协议暴露各种参数
 */

#include "register.hpp"
#include "register_handler.hpp"

namespace wibot::modbus::test {

// ============================================================================
// 寄存器定义 - 电源监控设备
// ============================================================================

// 输入参数 (可读写)
using SetVoltageReg = RegisterDef<0x0000,                          // 地址
                                  RegisterType::kHoldingRegister,  // 保持寄存器
                                  RegisterDataType::kUint16,       // u16类型
                                  RegisterAccess::kReadWrite,      // 可读写
                                  3300>;                           // 默认3.3V (单位: mV)

using SetCurrentReg = RegisterDef<0x0001, RegisterType::kHoldingRegister, RegisterDataType::kUint16,
                                  RegisterAccess::kReadWrite, 1000>;

// 测量值 (只读)
using MeasuredVoltageReg = RegisterDef<0x0100, RegisterType::kInputRegister,
                                       RegisterDataType::kUint16, RegisterAccess::kReadOnly, 0>;

using MeasuredCurrentReg = RegisterDef<0x0101, RegisterType::kInputRegister,
                                       RegisterDataType::kUint16, RegisterAccess::kReadOnly, 0>;

using TemperatureReg = RegisterDef<0x0102, RegisterType::kInputRegister, RegisterDataType::kInt16,
                                   RegisterAccess::kReadOnly, 250>;

// 功率 (32位，占用2个寄存器地址)
using PowerReg = RegisterDef<0x0103, RegisterType::kInputRegister, RegisterDataType::kUint32,
                             RegisterAccess::kReadOnly, 0>;

// 0x0104 被PowerReg占用，跳过

// 控制寄存器
using ControlReg = RegisterDef<0x0200, RegisterType::kHoldingRegister, RegisterDataType::kUint16,
                               RegisterAccess::kReadWrite, 0>;

using StatusReg = RegisterDef<0x0201, RegisterType::kHoldingRegister, RegisterDataType::kUint16,
                              RegisterAccess::kReadOnly, 0>;

// 校准系数 (浮点数)
using VoltageCalibrationReg = RegisterDef<0x0300, RegisterType::kHoldingRegister,
                                          RegisterDataType::kFloat, RegisterAccess::kReadWrite,
                                          0x3F800000>;  // 1.0

using CurrentCalibrationReg = RegisterDef<0x0302, RegisterType::kHoldingRegister,
                                          RegisterDataType::kFloat, RegisterAccess::kReadWrite,
                                          0x3F800000>;  // 1.0

// 设备信息 (只读)
using DeviceIdReg = RegisterDef<0x0400, RegisterType::kHoldingRegister, RegisterDataType::kUint16,
                                RegisterAccess::kReadOnly, 0x0001>;

using FirmwareVersionReg =
    RegisterDef<0x0401, RegisterType::kHoldingRegister, RegisterDataType::kUint16,
                RegisterAccess::kReadOnly, 0x0100>;

// ============================================================================
// 寄存器映射
// ============================================================================

using PowerMonitorRegisterMap =
    RegisterMap<SetVoltageReg, SetCurrentReg, MeasuredVoltageReg, MeasuredCurrentReg,
                TemperatureReg, PowerReg, ControlReg, StatusReg, VoltageCalibrationReg,
                CurrentCalibrationReg, DeviceIdReg, FirmwareVersionReg>;

// ============================================================================
// 设备控制器
// ============================================================================

class PowerMonitorController {
   public:
    // 控制位定义
    enum ControlBits : u16 {
        kOutputEnable = 0x0001,
        kOvpEnable    = 0x0002,
        kOcpEnable    = 0x0004,
        kReset        = 0x8000
    };

    // 状态位定义
    enum StatusBits : u16 {
        kOutputOn   = 0x0001,
        kOvpTripped = 0x0002,
        kOcpTripped = 0x0004,
        kOverTemp   = 0x0008,
        kReady      = 0x0100,
        kError      = 0x8000
    };

   public:
    PowerMonitorController() {
        // 寄存器已自动初始化为默认值
        updateStatus();
    }

    // ========================================================================
    // 设置值控制
    // ========================================================================

    void setTargetVoltage(u16 millivolts) {
        _registers.set<SetVoltageReg>(millivolts);
        applySettings();
    }

    u16 getTargetVoltage() const {
        return _registers.read<SetVoltageReg>();
    }

    void setTargetCurrent(u16 milliamps) {
        _registers.set<SetCurrentReg>(milliamps);
        applySettings();
    }

    u16 getTargetCurrent() const {
        return _registers.read<SetCurrentReg>();
    }

    // ========================================================================
    // 测量值更新 (由硬件/传感器更新)
    // ========================================================================

    void updateMeasuredVoltage(u16 millivolts) {
        _registers.get<MeasuredVoltageReg>().value() = millivolts;
        updatePower();
    }

    void updateMeasuredCurrent(u16 milliamps) {
        _registers.get<MeasuredCurrentReg>().value() = milliamps;
        updatePower();
    }

    void updateTemperature(i16 tempTenth) {
        _registers.get<TemperatureReg>().value() = tempTenth;
        checkOverTemp();
    }

    // ========================================================================
    // 控制操作
    // ========================================================================

    void enableOutput(bool enable) {
        u16 ctrl = _registers.read<ControlReg>();
        if (enable) {
            ctrl |= kOutputEnable;
        } else {
            ctrl &= ~kOutputEnable;
        }
        _registers.set<ControlReg>(ctrl);
        applySettings();
    }

    bool isOutputEnabled() const {
        return (_registers.read<ControlReg>() & kOutputEnable) != 0;
    }

    void reset() {
        u16 ctrl = _registers.read<ControlReg>();
        ctrl |= kReset;
        _registers.set<ControlReg>(ctrl);
        applySettings();

        // 清除复位位
        ctrl &= ~kReset;
        _registers.set<ControlReg>(ctrl);
    }

    // ========================================================================
    // 校准
    // ========================================================================

    void setVoltageCalibration(f32 factor) {
        _registers.set<VoltageCalibrationReg>(factor);
    }

    f32 getVoltageCalibration() const {
        return _registers.read<VoltageCalibrationReg>();
    }

    void setCurrentCalibration(f32 factor) {
        _registers.set<CurrentCalibrationReg>(factor);
    }

    f32 getCurrentCalibration() const {
        return _registers.read<CurrentCalibrationReg>();
    }

    // ========================================================================
    // 状态查询
    // ========================================================================

    bool isReady() const {
        return (_registers.read<StatusReg>() & kReady) != 0;
    }

    bool hasError() const {
        return (_registers.read<StatusReg>() & kError) != 0;
    }

    u16 getStatus() const {
        return _registers.read<StatusReg>();
    }

    // ========================================================================
    // Modbus接口
    // ========================================================================

    PowerMonitorRegisterMap& getRegisterMap() {
        return _registers;
    }

    const PowerMonitorRegisterMap& getRegisterMap() const {
        return _registers;
    }

    // 处理Modbus读保持寄存器请求
    bool handleReadHoldingRegisters(u16 addr, u16 count, u8* response) {
// 简化实现，实际应用需要处理多个寄存器的连续读取

// 使用宏来简化代码
#define HANDLE_READ_REG(RegType)                                                       \
    if (addr == RegType::Address && RegType::Type == RegisterType::kHoldingRegister && \
        count == RegType::RegisterCount) {                                             \
        _registers.writeToBuffer<RegType>(response);                                   \
        return true;                                                                   \
    }

        HANDLE_READ_REG(SetVoltageReg)
        HANDLE_READ_REG(SetCurrentReg)
        HANDLE_READ_REG(ControlReg)
        HANDLE_READ_REG(StatusReg)
        HANDLE_READ_REG(VoltageCalibrationReg)
        HANDLE_READ_REG(CurrentCalibrationReg)
        HANDLE_READ_REG(DeviceIdReg)
        HANDLE_READ_REG(FirmwareVersionReg)

#undef HANDLE_READ_REG

        return false;  // 地址不存在或类型不匹配
    }

    // 处理Modbus读输入寄存器请求
    bool handleReadInputRegisters(u16 addr, u16 count, u8* response) {
#define HANDLE_READ_REG(RegType)                                                     \
    if (addr == RegType::Address && RegType::Type == RegisterType::kInputRegister && \
        count == RegType::RegisterCount) {                                           \
        _registers.writeToBuffer<RegType>(response);                                 \
        return true;                                                                 \
    }

        HANDLE_READ_REG(MeasuredVoltageReg)
        HANDLE_READ_REG(MeasuredCurrentReg)
        HANDLE_READ_REG(TemperatureReg)
        HANDLE_READ_REG(PowerReg)

#undef HANDLE_READ_REG

        return false;
    }

    // 处理Modbus写保持寄存器请求
    bool handleWriteHoldingRegisters(u16 addr, u16 count, const u8* data) {
#define HANDLE_WRITE_REG(RegType)                                                          \
    if (addr == RegType::Address && RegType::Type == RegisterType::kHoldingRegister &&     \
        RegType::Access != RegisterAccess::kReadOnly && count == RegType::RegisterCount) { \
        _registers.readFromBuffer<RegType>(data);                                          \
        applySettings();                                                                   \
        return true;                                                                       \
    }

        HANDLE_WRITE_REG(SetVoltageReg)
        HANDLE_WRITE_REG(SetCurrentReg)
        HANDLE_WRITE_REG(ControlReg)
        HANDLE_WRITE_REG(VoltageCalibrationReg)
        HANDLE_WRITE_REG(CurrentCalibrationReg)

#undef HANDLE_WRITE_REG

        return false;
    }

   private:
    void updatePower() {
        u16 voltage                        = _registers.read<MeasuredVoltageReg>();
        u16 current                        = _registers.read<MeasuredCurrentReg>();
        u32 power                          = static_cast<u32>(voltage) * current / 1000;  // mW
        _registers.get<PowerReg>().value() = power;
    }

    void updateStatus() {
        u16 status = 0;

        if (isOutputEnabled()) {
            status |= kOutputOn;
        }

        // 检查各种条件...
        status |= kReady;  // 简化示例

        _registers.get<StatusReg>().value() = status;
    }

    void checkOverTemp() {
        i16 temp = _registers.read<TemperatureReg>();
        if (temp > 850) {  // 85.0℃
            u16 status = _registers.read<StatusReg>();
            status |= kOverTemp;
            _registers.get<StatusReg>().value() = status;
        }
    }

    void applySettings() {
        // 这里应该将寄存器值应用到实际硬件
        // ...

        updateStatus();
    }

   private:
    PowerMonitorRegisterMap _registers;
};

// ============================================================================
// 测试函数
// ============================================================================

inline void runRegisterTest() {
    PowerMonitorController controller;

    // 1. 基本设置
    controller.setTargetVoltage(5000);  // 5V
    controller.setTargetCurrent(2000);  // 2A

    // 2. 启用输出
    controller.enableOutput(true);

    // 3. 模拟测量值更新
    controller.updateMeasuredVoltage(4980);  // 4.98V
    controller.updateMeasuredCurrent(1950);  // 1.95A
    controller.updateTemperature(320);       // 32.0℃

    // 4. 读取状态
    u16  status = controller.getStatus();
    bool ready  = controller.isReady();
    (void)status;  // 避免未使用警告
    (void)ready;

    // 5. 校准
    controller.setVoltageCalibration(1.01f);
    controller.setCurrentCalibration(0.99f);

    // 6. Modbus通信测试
    u8 buffer[8];

    // 读取设定电压
    if (controller.handleReadHoldingRegisters(0x0000, 1, buffer)) {
        // buffer[0] = 0x13, buffer[1] = 0x88 (5000)
    }

    // 读取测量电流
    if (controller.handleReadInputRegisters(0x0101, 1, buffer)) {
        // buffer[0] = 0x07, buffer[1] = 0x9E (1950)
    }

    // 写入新的电压设定
    buffer[0] = 0x0C;
    buffer[1] = 0xE4;  // 3300 (3.3V)
    if (controller.handleWriteHoldingRegisters(0x0000, 1, buffer)) {
        // 设定值已更新
    }

    // 7. 测试32位寄存器
    if (controller.handleReadInputRegisters(0x0103, 2, buffer)) {
        // 读取功率值 (占用2个寄存器)
        // buffer[0-3]包含32位功率值
    }

    // 8. 测试浮点寄存器
    if (controller.handleReadHoldingRegisters(0x0300, 2, buffer)) {
        // 读取电压校准系数 (占用2个寄存器)
        // buffer[0-3]包含IEEE 754浮点数
    }
}

}  // namespace wibot::modbus::test

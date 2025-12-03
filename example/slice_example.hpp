#pragma once

#include "register.hpp"

/**
 * @file slice_example.hpp
 * @brief 使用现有Slice基础设施的Modbus寄存器示例
 * 
 * 展示如何利用现有的Buffer和Slice来进行寄存器操作
 */

namespace wibot::modbus::slice_example {

// ============================================================================
// 寄存器定义
// ============================================================================

// 基本寄存器
using VoltageReg = RegisterDef<0x0000, RegisterType::kHoldingRegister, RegisterDataType::kUint16,
                               RegisterAccess::kReadWrite, 3300>;
using CurrentReg = RegisterDef<0x0001, RegisterType::kHoldingRegister, RegisterDataType::kUint16,
                               RegisterAccess::kReadWrite, 1000>;
using TemperatureReg = RegisterDef<0x0002, RegisterType::kInputRegister, RegisterDataType::kInt16,
                                   RegisterAccess::kReadOnly, 250>;

// 32位寄存器
using PowerReg = RegisterDef<0x0003, RegisterType::kInputRegister, RegisterDataType::kUint32,
                             RegisterAccess::kReadOnly, 0>;

// 浮点寄存器
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
// 设备控制器 - 展示Slice集成
// ============================================================================

class SliceDevice {
   public:
    SliceDevice() {
        // 寄存器自动初始化为默认值
    }

    // ========================================================================
    // 基础操作
    // ========================================================================

    void setVoltage(u16 mv) {
        registers_.set<VoltageReg>(mv);
    }

    u16 getVoltage() const {
        return registers_.read<VoltageReg>();
    }

    void updateTemperature(i16 temp) {
        registers_.get<TemperatureReg>().value() = temp;
    }

    void updatePower(u32 power) {
        registers_.get<PowerReg>().value() = power;
    }

    // ========================================================================
    // Slice操作示例
    // ========================================================================

    /**
     * @brief 将单个寄存器写入Slice
     */
    void writeVoltageToSlice(Slice& slice, u16 offset = 0) const {
        registers_.writeToSlice<VoltageReg>(slice, offset);
    }

    /**
     * @brief 从Slice读取单个寄存器
     */
    void readVoltageFromSlice(const Slice& slice, u16 offset = 0) {
        registers_.readFromSlice<VoltageReg>(slice, offset);
    }

    /**
     * @brief 批量写入多个寄存器到Slice
     */
    void writeAllMeasurementsToSlice(Slice& slice) const {
        registers_.writeRegistersToSlice<VoltageReg, CurrentReg, TemperatureReg, PowerReg>(slice);
    }

    /**
     * @brief 批量从Slice读取多个寄存器
     */
    void readAllMeasurementsFromSlice(const Slice& slice) {
        registers_.readRegistersFromSlice<VoltageReg, CurrentReg, TemperatureReg, PowerReg>(slice);
    }

    // ========================================================================
    // Modbus 报文处理示例
    // ========================================================================

    /**
     * @brief 处理Modbus读保持寄存器请求
     * @param startAddr 起始地址
     * @param count 寄存器数量  
     * @param responseBuffer 响应缓冲区
     * @return true 成功, false 失败
     */
    bool handleReadHoldingRegisters(u16 startAddr, u16 count, Buffer<256>& responseBuffer) {
        // 简化示例：只处理连续读取
        if (startAddr == VoltageReg::Address && count == 2) {
            // 读取电压和电流寄存器
            Slice responseSlice = responseBuffer;  // 自动转换为Slice

            registers_.writeToSlice<VoltageReg>(responseSlice, 0);
            registers_.writeToSlice<CurrentReg>(responseSlice, 2);

            responseBuffer.size = 4;  // 设置实际数据长度
            return true;
        }

        if (startAddr == CalibrationReg::Address && count == 2) {
            // 读取32位浮点校准值
            Slice responseSlice = responseBuffer;
            registers_.writeToSlice<CalibrationReg>(responseSlice, 0);

            responseBuffer.size = 4;  // 32位数据占4字节
            return true;
        }

        return false;  // 不支持的地址范围
    }

    /**
     * @brief 处理Modbus写保持寄存器请求
     */
    bool handleWriteHoldingRegisters(u16 startAddr, u16 count, const Slice& requestData) {
        if (startAddr == VoltageReg::Address && count == 1) {
            registers_.readFromSlice<VoltageReg>(requestData, 0);
            return true;
        }

        if (startAddr == CurrentReg::Address && count == 1) {
            registers_.readFromSlice<CurrentReg>(requestData, 0);
            return true;
        }

        if (startAddr == CalibrationReg::Address && count == 2) {
            registers_.readFromSlice<CalibrationReg>(requestData, 0);
            return true;
        }

        return false;
    }

    // ========================================================================
    // 高级功能：支持不同字节序
    // ========================================================================

    /**
     * @brief 写入寄存器到Slice，支持指定字节序
     */
    template <typename RegDef>
    void writeToSliceWithEndian(Slice& slice, u16 offset, Endian endian) const {
        const auto& regItem = registers_.get<RegDef>();
        auto        value   = regItem.get();

        if constexpr (RegDef::DataType == RegisterDataType::kUint16) {
            slice.setUint16(offset, value, endian);
        } else if constexpr (RegDef::DataType == RegisterDataType::kInt16) {
            slice.setInt16(offset, value, endian);
        } else if constexpr (RegDef::DataType == RegisterDataType::kUint32) {
            slice.setUint32(offset, value, endian);
        } else if constexpr (RegDef::DataType == RegisterDataType::kInt32) {
            slice.setInt32(offset, value, endian);
        } else if constexpr (RegDef::DataType == RegisterDataType::kFloat) {
            slice.setFloat(offset, value, endian);
        }
    }

    /**
     * @brief 支持小端序的写入（用于某些特殊设备）
     */
    void writeVoltageToSliceLittleEndian(Slice& slice, u16 offset = 0) const {
        writeToSliceWithEndian<VoltageReg>(slice, offset, Endian::kLittle);
    }

    // ========================================================================
    // 数据导出功能
    // ========================================================================

    /**
     * @brief 导出所有寄存器到固定大小的Buffer
     */
    void exportAllRegisters(Buffer<32>& exportBuffer) const {
        Slice exportSlice = exportBuffer;
        u16   offset      = 0;

        // 按地址顺序导出
        registers_.writeToSlice<VoltageReg>(exportSlice, offset);
        offset += 2;
        registers_.writeToSlice<CurrentReg>(exportSlice, offset);
        offset += 2;
        registers_.writeToSlice<TemperatureReg>(exportSlice, offset);
        offset += 2;
        registers_.writeToSlice<PowerReg>(exportSlice, offset);
        offset += 4;  // 32位
        registers_.writeToSlice<CalibrationReg>(exportSlice, offset);
        offset += 4;  // 32位浮点
        registers_.writeToSlice<StatusReg>(exportSlice, offset);
        offset += 2;

        exportBuffer.size = offset;  // 设置实际使用的字节数
    }

    /**
     * @brief 从Buffer导入所有寄存器
     */
    void importAllRegisters(const Buffer<32>& importBuffer) {
        Slice importSlice(const_cast<u8*>(importBuffer.data), importBuffer.size);
        u16   offset = 0;

        registers_.readFromSlice<VoltageReg>(importSlice, offset);
        offset += 2;
        registers_.readFromSlice<CurrentReg>(importSlice, offset);
        offset += 2;
        registers_.readFromSlice<TemperatureReg>(importSlice, offset);
        offset += 2;
        registers_.readFromSlice<PowerReg>(importSlice, offset);
        offset += 4;
        registers_.readFromSlice<CalibrationReg>(importSlice, offset);
        offset += 4;
        registers_.readFromSlice<StatusReg>(importSlice, offset);
        offset += 2;
    }

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

inline void runSliceExample() {
    SliceDevice device;

    // 1. 基础设置
    device.setVoltage(5000);        // 5V
    device.updateTemperature(320);  // 32.0℃
    device.updatePower(9750);       // 9.75W (5V * 1.95A)

    // 2. 单个寄存器的Slice操作
    Buffer<8> buffer;
    Slice     slice = buffer;

    // 写入电压到Slice
    device.writeVoltageToSlice(slice, 0);
    // slice现在包含: [0x13, 0x88] (5000的大端表示)

    // 从Slice读取电压
    device.readVoltageFromSlice(slice, 0);

    // 3. 批量操作
    Buffer<16> measurementBuffer;
    Slice      measurementSlice = measurementBuffer;

    // 批量写入所有测量值
    device.writeAllMeasurementsToSlice(measurementSlice);
    // 缓冲区现在包含：电压(2字节) + 电流(2字节) + 温度(2字节) + 功率(4字节)

    // 批量读取
    device.readAllMeasurementsFromSlice(measurementSlice);

    // 4. Modbus通信示例
    Buffer<256> modbusResponse;
    bool        success = device.handleReadHoldingRegisters(0x0000, 2, modbusResponse);
    if (success) {
        // modbusResponse包含电压和电流的Modbus响应
    }

    // 5. 字节序控制
    Buffer<4> littleEndianBuffer;
    Slice     littleEndianSlice = littleEndianBuffer;
    device.writeVoltageToSliceLittleEndian(littleEndianSlice, 0);
    // 现在包含: [0x88, 0x13] (5000的小端表示)

    // 6. 完整数据导出/导入
    Buffer<32> exportBuffer;
    device.exportAllRegisters(exportBuffer);

    SliceDevice device2;
    device2.importAllRegisters(exportBuffer);
    // device2现在包含与device相同的寄存器值

    // 7. 验证数据一致性
    bool dataMatches = (device.getVoltage() == device2.getVoltage());
    (void)dataMatches;  // 避免未使用警告
}

}  // namespace wibot::modbus::slice_example
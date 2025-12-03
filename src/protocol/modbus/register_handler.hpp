#pragma once

#include "register.hpp"
#include "../../base/buffer.hpp"

namespace wibot::modbus {

/**
 * @brief Modbus从机寄存器处理器
 * 
 * 这个类负责处理Modbus从机的寄存器读写请求，
 * 使用零开销的模板机制进行寄存器访问
 * 
 * @tparam RegMap 寄存器映射类型 (RegisterMap<...>)
 */
template <typename RegMap>
class ModbusSlaveRegisterHandler {
   public:
    ModbusSlaveRegisterHandler(RegMap& registerMap) : _registerMap(registerMap) {
    }

    /**
     * @brief 读取保持寄存器到缓冲区
     * @param startAddr 起始地址
     * @param count 寄存器数量
     * @param buffer 输出缓冲区
     * @return true 成功, false 失败
     */
    bool readHoldingRegisters(u16 startAddr, u16 count, u8* buffer) {
        return readRegistersGeneric<RegisterType::kHoldingRegister>(startAddr, count, buffer);
    }

    /**
     * @brief 读取输入寄存器到缓冲区
     * @param startAddr 起始地址
     * @param count 寄存器数量
     * @param buffer 输出缓冲区
     * @return true 成功, false 失败
     */
    bool readInputRegisters(u16 startAddr, u16 count, u8* buffer) {
        return readRegistersGeneric<RegisterType::kInputRegister>(startAddr, count, buffer);
    }

    /**
     * @brief 从缓冲区写入保持寄存器
     * @param startAddr 起始地址
     * @param count 寄存器数量
     * @param buffer 输入缓冲区
     * @return true 成功, false 失败
     */
    bool writeHoldingRegisters(u16 startAddr, u16 count, const u8* buffer) {
        return writeRegistersGeneric<RegisterType::kHoldingRegister>(startAddr, count, buffer);
    }

    /**
     * @brief 通过类型直接读取寄存器值
     */
    template <typename RegDef>
    auto readRegister() const {
        return _registerMap.template read<RegDef>();
    }

    /**
     * @brief 通过类型直接写入寄存器值
     */
    template <typename RegDef>
    void writeRegister(typename RegisterItem<RegDef>::ValueType value) {
        _registerMap.template set<RegDef>(value);
    }

    /**
     * @brief 通过类型将寄存器写入缓冲区
     */
    template <typename RegDef>
    void writeRegisterToBuffer(u8* buffer) const {
        _registerMap.template writeToBuffer<RegDef>(buffer);
    }

    /**
     * @brief 通过类型从缓冲区读取寄存器
     */
    template <typename RegDef>
    void readRegisterFromBuffer(const u8* buffer) {
        _registerMap.template readFromBuffer<RegDef>(buffer);
    }

    RegMap& getRegisterMap() {
        return _registerMap;
    }
    const RegMap& getRegisterMap() const {
        return _registerMap;
    }

   private:
    template <RegisterType TYPE>
    bool readRegistersGeneric(u16 startAddr, u16 count, u8* buffer) {
        // 这是一个简化实现，实际应用中可能需要更复杂的逻辑
        // 根据寄存器映射动态查找对应的寄存器
        // 由于我们使用编译期类型，运行时查找需要一些技巧

        // 为了保持零开销，建议使用特化的模板函数为每个寄存器生成代码
        // 或者提供回调机制

        u16 bufferOffset = 0;
        for (u16 i = 0; i < count; i++) {
            u16 addr = startAddr + i;

            // 这里需要遍历所有寄存器来查找匹配的地址
            // 可以通过宏或辅助函数生成
            bool found = readRegisterByAddress<TYPE>(addr, buffer + bufferOffset);

            if (!found) {
                return false;  // 寄存器不存在
            }

            bufferOffset += 2;  // 每个寄存器2字节
        }

        return true;
    }

    template <RegisterType TYPE>
    bool writeRegistersGeneric(u16 startAddr, u16 count, const u8* buffer) {
        u16 bufferOffset = 0;
        for (u16 i = 0; i < count; i++) {
            u16 addr = startAddr + i;

            bool found = writeRegisterByAddress<TYPE>(addr, buffer + bufferOffset);

            if (!found) {
                return false;
            }

            bufferOffset += 2;
        }

        return true;
    }

    // 这些函数需要在具体应用中特化实现
    template <RegisterType TYPE>
    bool readRegisterByAddress(u16 addr, u8* buffer) {
        // 默认返回false，表示寄存器不存在
        // 在实际使用中，应该通过宏或代码生成为每个寄存器类型生成对应的分支
        return false;
    }

    template <RegisterType TYPE>
    bool writeRegisterByAddress(u16 addr, const u8* buffer) {
        return false;
    }

    RegMap& _registerMap;
};

/**
 * @brief 寄存器访问助手宏
 * 
 * 用于生成运行时地址到编译期类型的映射
 */
#define MODBUS_REGISTER_CASE(RegDef, addr, buffer, handler, operation) \
    if (addr == RegDef::Address && RegDef::Type == TYPE) {             \
        handler.template operation<RegDef>(buffer);                    \
        return true;                                                   \
    }

/**
 * @brief 为寄存器处理器生成地址查找代码的宏
 * 
 * 使用示例:
 * MODBUS_REGISTER_MAP_BEGIN(MyHandler, MyRegisterMap)
 *     MODBUS_REGISTER_ENTRY(VoltageReg)
 *     MODBUS_REGISTER_ENTRY(CurrentReg)
 *     MODBUS_REGISTER_ENTRY(TemperatureReg)
 * MODBUS_REGISTER_MAP_END()
 */
#define MODBUS_REGISTER_MAP_BEGIN(HandlerClass, RegMapType) \
    template <>                                             \
    template <RegisterType TYPE>                            \
    bool ModbusSlaveRegisterHandler<RegMapType>::readRegisterByAddress(u16 addr, u8* buffer) {
#define MODBUS_REGISTER_ENTRY(RegDef) \
    MODBUS_REGISTER_CASE(RegDef, addr, buffer, _registerMap, writeToBuffer)

#define MODBUS_REGISTER_MAP_END()                                                           \
    return false;                                                                           \
    }                                                                                       \
    template <>                                                                             \
    template <RegisterType TYPE>                                                            \
    bool ModbusSlaveRegisterHandler<RegMapType>::writeRegisterByAddress(u16       addr,     \
                                                                        const u8* buffer) { \
        /* 这里可以为写操作添加类似的ENTRY */                                               \
        return false;                                                                       \
    }

/**
 * @brief 简化的寄存器访问器 - 完全编译期确定
 * 
 * 当你知道确切的寄存器类型时，直接使用这个类可以获得最佳性能
 * 
 * @tparam RegMap 寄存器映射类型
 */
template <typename RegMap>
class DirectRegisterAccessor {
   public:
    DirectRegisterAccessor(RegMap& registerMap) : _registerMap(registerMap) {
    }

    /**
     * @brief 直接读取寄存器 (零运行时开销)
     */
    template <typename RegDef>
    auto read() const {
        return _registerMap.template read<RegDef>();
    }

    /**
     * @brief 直接写入寄存器 (零运行时开销)
     */
    template <typename RegDef>
    void write(typename RegisterItem<RegDef>::ValueType value) {
        _registerMap.template set<RegDef>(value);
    }

    /**
     * @brief 获取寄存器引用 (零运行时开销)
     */
    template <typename RegDef>
    auto& get() {
        return _registerMap.template get<RegDef>();
    }

   private:
    RegMap& _registerMap;
};

}  // namespace wibot::modbus

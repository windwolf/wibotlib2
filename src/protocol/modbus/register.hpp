#pragma once

#include "../../base/type.hpp"
#include "../../base/buffer.hpp"
#include <tuple>
#include <type_traits>

namespace wibot::modbus {

/**
 * @brief Modbus设备模式
 */
enum class ModbusMode : u8 {
    kMaster,  // 主设备模式 - 主动发起请求，维护远程寄存器镜像
    kSlave    // 从设备模式 - 被动响应请求，通过地址索引寄存器
};

/**
 * @brief Modbus寄存器类型
 */
enum class RegisterType : u8 {
    kHoldingRegister,  // 保持寄存器 (可读写)
    kInputRegister,    // 输入寄存器 (只读)
    kCoil,             // 线圈 (可读写)
    kDiscreteInput     // 离散输入 (只读)
};

/**
 * @brief 寄存器访问权限
 */
enum class RegisterAccess : u8 {
    kReadOnly,
    kWriteOnly,
    kReadWrite
};

/**
 * @brief 寄存器数据类型
 */
enum class RegisterDataType : u8 {
    kUint16,  // 16位无符号整数
    kInt16,   // 16位有符号整数
    kUint32,  // 32位无符号整数 (占用2个寄存器)
    kInt32,   // 32位有符号整数 (占用2个寄存器)
    kFloat    // 32位浮点数 (占用2个寄存器)
};

/**
 * @brief 寄存器定义
 * @tparam ADDR 寄存器地址
 * @tparam TYPE 寄存器类型
 * @tparam DATA_TYPE 数据类型
 * @tparam ACCESS 访问权限
 * @tparam DEFAULT 默认值
 */
template <u16 ADDR, RegisterType TYPE = RegisterType::kHoldingRegister,
          RegisterDataType DATA_TYPE = RegisterDataType::kUint16,
          RegisterAccess ACCESS = RegisterAccess::kReadWrite, u32 DEFAULT = 0>
struct RegisterDef {
    static constexpr u16              Address      = ADDR;
    static constexpr RegisterType     Type         = TYPE;
    static constexpr RegisterDataType DataType     = DATA_TYPE;
    static constexpr RegisterAccess   Access       = ACCESS;
    static constexpr u32              DefaultValue = DEFAULT;

    // 根据数据类型确定占用的寄存器数量
    static constexpr u16 RegisterCount =
        (DATA_TYPE == RegisterDataType::kUint16 || DATA_TYPE == RegisterDataType::kInt16) ? 1 : 2;
};

// ============================================================================
// 根据数据类型获取ValueType (使用std::conditional_t)
// ============================================================================

template <typename RegDef>
using RegisterValueType_t = std::conditional_t<
    RegDef::DataType == RegisterDataType::kUint16, u16,
    std::conditional_t<
        RegDef::DataType == RegisterDataType::kInt16, i16,
        std::conditional_t<
            RegDef::DataType == RegisterDataType::kUint32, u32,
            std::conditional_t<RegDef::DataType == RegisterDataType::kInt32, i32, f32>>>>;

/**
 * @brief 寄存器项 - 运行时数据存储
 * @tparam RegDef 寄存器定义
 */
template <typename RegDef>
class RegisterItem {
   public:
    using ValueType = RegisterValueType_t<RegDef>;

    RegisterItem() : _value(static_cast<ValueType>(RegDef::DefaultValue)) {
    }

    // 设置值
    void set(ValueType value) {
        static_assert(RegDef::Access == RegisterAccess::kWriteOnly ||
                          RegDef::Access == RegisterAccess::kReadWrite,
                      "Register is read-only");
        _value = value;
    }

    // 获取值
    ValueType get() const {
        static_assert(RegDef::Access == RegisterAccess::kReadOnly ||
                          RegDef::Access == RegisterAccess::kReadWrite,
                      "Register is write-only");
        return _value;
    }

    // 直接访问值（用于内部操作，跳过权限检查）
    ValueType& value() {
        return _value;
    }
    const ValueType& value() const {
        return _value;
    }

    // 写入到缓冲区 (使用现有的Slice API)
    void writeToBuffer(u8* buffer) const {
        Slice slice(buffer, getBufferSize());
        writeToSlice(slice);
    }

    // 写入到Slice (支持大小端转换)
    void writeToSlice(Slice& slice, u16 offset = 0) const {
        if constexpr (RegDef::DataType == RegisterDataType::kUint16) {
            slice.setUint16(offset, _value, Endian::kBig);
        } else if constexpr (RegDef::DataType == RegisterDataType::kInt16) {
            slice.setInt16(offset, _value, Endian::kBig);
        } else if constexpr (RegDef::DataType == RegisterDataType::kUint32) {
            slice.setUint32(offset, _value, Endian::kBig);
        } else if constexpr (RegDef::DataType == RegisterDataType::kInt32) {
            slice.setInt32(offset, _value, Endian::kBig);
        } else if constexpr (RegDef::DataType == RegisterDataType::kFloat) {
            slice.setFloat(offset, _value, Endian::kBig);
        }
    }

    // 从缓冲区读取 (使用现有的Slice API)
    void readFromBuffer(const u8* buffer) {
        Slice slice(const_cast<u8*>(buffer), getBufferSize());
        readFromSlice(slice);
    }

    // 从Slice读取 (支持大小端转换)
    void readFromSlice(const Slice& slice, u16 offset = 0) {
        if constexpr (RegDef::DataType == RegisterDataType::kUint16) {
            _value = slice.getUint16(offset, Endian::kBig);
        } else if constexpr (RegDef::DataType == RegisterDataType::kInt16) {
            _value = slice.getInt16(offset, Endian::kBig);
        } else if constexpr (RegDef::DataType == RegisterDataType::kUint32) {
            _value = slice.getUint32(offset, Endian::kBig);
        } else if constexpr (RegDef::DataType == RegisterDataType::kInt32) {
            _value = slice.getInt32(offset, Endian::kBig);
        } else if constexpr (RegDef::DataType == RegisterDataType::kFloat) {
            _value = slice.getFloat(offset, Endian::kBig);
        }
    }

    // 获取缓冲区大小
    static constexpr u16 getBufferSize() {
        return RegDef::RegisterCount * 2;  // 每个寄存器2字节
    }

   private:
    ValueType _value;
};

/**
 * @brief Master寄存器映射 - 编译期构建的寄存器集合，维护远程设备寄存器镜像
 * @tparam RegDefs 寄存器定义列表
 */
template <typename... RegDefs>
class MasterRegisterMap {
   public:
    MasterRegisterMap() {
        // 初始化所有寄存器为默认值
        (_initRegister<RegDefs>(), ...);
    }

    /**
     * @brief 通过类型获取寄存器（编译期确定，零运行时开销）
     */
    template <typename RegDef>
    auto& get() {
        return std::get<RegisterItem<RegDef>>(_registers);
    }

    template <typename RegDef>
    const auto& get() const {
        return std::get<RegisterItem<RegDef>>(_registers);
    }

    /**
     * @brief 设置寄存器值
     */
    template <typename RegDef>
    void set(typename RegisterItem<RegDef>::ValueType value) {
        get<RegDef>().set(value);
    }

    /**
     * @brief 读取寄存器值
     */
    template <typename RegDef>
    auto read() const {
        return get<RegDef>().get();
    }

    /**
     * @brief 将寄存器数据写入缓冲区
     * @tparam RegDef 寄存器定义
     * @param buffer 目标缓冲区
     */
    template <typename RegDef>
    void writeToBuffer(u8* buffer) const {
        get<RegDef>().writeToBuffer(buffer);
    }

    /**
     * @brief 从缓冲区读取数据到寄存器
     * @tparam RegDef 寄存器定义
     * @param buffer 源缓冲区
     */
    template <typename RegDef>
    void readFromBuffer(const u8* buffer) {
        get<RegDef>().readFromBuffer(buffer);
    }

    /**
     * @brief 写入寄存器到Slice
     * @tparam RegDef 寄存器定义
     * @param slice 目标Slice
     * @param offset 偏移量
     */
    template <typename RegDef>
    void writeToSlice(Slice& slice, u16 offset = 0) const {
        get<RegDef>().writeToSlice(slice, offset);
    }

    /**
     * @brief 从Slice读取数据到寄存器
     * @tparam RegDef 寄存器定义
     * @param slice 源Slice
     * @param offset 偏移量
     */
    template <typename RegDef>
    void readFromSlice(const Slice& slice, u16 offset = 0) {
        get<RegDef>().readFromSlice(slice, offset);
    }

    /**
     * @brief 批量写入多个寄存器到Slice
     */
    template <typename... SelectedRegDefs>
    void writeRegistersToSlice(Slice& slice) const {
        u16 offset = 0;
        (writeRegisterWithOffset<SelectedRegDefs>(slice, offset), ...);
    }

    /**
     * @brief 批量从一个Slice读取多个寄存器
     */
    template <typename... SelectedRegDefs>
    void readRegistersFromSlice(const Slice& slice) {
        u16 offset = 0;
        (readRegisterWithOffset<SelectedRegDefs>(slice, offset), ...);
    }

    /**
     * @brief 获取寄存器总数
     */
    static constexpr size_t size() {
        return sizeof...(RegDefs);
    }

   private:
    // 批量操作的辅助方法
    template <typename RegDef>
    void writeRegisterWithOffset(Slice& slice, u16& offset) const {
        get<RegDef>().writeToSlice(slice, offset);
        offset += RegDef::RegisterCount * 2;  // 更新偏移量
    }

    template <typename RegDef>
    void readRegisterWithOffset(const Slice& slice, u16& offset) {
        get<RegDef>().readFromSlice(slice, offset);
        offset += RegDef::RegisterCount * 2;  // 更新偏移量
    }
    template <typename RegDef>
    void _initRegister() {
        // 编译期验证寄存器地址不重复
        static_assert(checkNoDuplicateAddress<RegDefs...>(), "Duplicate register address detected");

        // 检查当前寄存器是否与其他寄存器有地址范围重叠
        static_assert(checkNoAddressOverlap<RegDefs...>(),
                      "Register address range overlaps with other registers");
    }

    // 编译期检查地址冲突 - 检查所有寄存器定义中是否有重复地址
    template <typename... AllRegDefs>
    static constexpr bool checkNoDuplicateAddress() {
        return checkNoDuplicateAddressImpl<AllRegDefs...>();
    }

    // 递归检查实现
    template <typename First, typename... Rest>
    static constexpr bool checkNoDuplicateAddressImpl() {
        if constexpr (sizeof...(Rest) == 0) {
            return true;  // 只有一个寄存器，没有重复
        } else {
            return !hasAddressConflict<First, Rest...>() && checkNoDuplicateAddressImpl<Rest...>();
        }
    }

    // 检查一个寄存器是否与其他寄存器地址冲突
    template <typename Check, typename... Others>
    static constexpr bool hasAddressConflict() {
        return ((Check::Address == Others::Address) || ...);
    }

    // 检查地址范围重叠（考虑32位数据类型占用2个寄存器）
    template <typename... AllRegDefs>
    static constexpr bool checkNoAddressOverlap() {
        return checkNoAddressOverlapImpl<AllRegDefs...>();
    }

    template <typename First, typename... Rest>
    static constexpr bool checkNoAddressOverlapImpl() {
        if constexpr (sizeof...(Rest) == 0) {
            return true;
        } else {
            return !hasAddressRangeOverlap<First, Rest...>() &&
                   checkNoAddressOverlapImpl<Rest...>();
        }
    }

    // 检查一个寄存器的地址范围是否与其他寄存器重叠
    template <typename Check, typename... Others>
    static constexpr bool hasAddressRangeOverlap() {
        return (addressRangeOverlaps<Check, Others>() || ...);
    }

    // 检查两个寄存器的地址范围是否重叠
    template <typename Reg1, typename Reg2>
    static constexpr bool addressRangeOverlaps() {
        constexpr u16 reg1_start = Reg1::Address;
        constexpr u16 reg1_end   = Reg1::Address + Reg1::RegisterCount - 1;
        constexpr u16 reg2_start = Reg2::Address;
        constexpr u16 reg2_end   = Reg2::Address + Reg2::RegisterCount - 1;

        // 检查两个范围是否重叠
        return !(reg1_end < reg2_start || reg2_end < reg1_start);
    }

    std::tuple<RegisterItem<RegDefs>...> _registers;
};

/**
 * @brief Slave寄存器映射 - 通过地址快速索引寄存器，用于响应Modbus请求
 * @tparam RegDefs 寄存器定义列表
 */
template <typename... RegDefs>
class SlaveRegisterMap {
   public:
    SlaveRegisterMap() {
        // 初始化所有寄存器为默认值
        (_initRegister<RegDefs>(), ...);
        // 构建地址索引表
        _buildAddressIndex();
    }

    /**
     * @brief 通过地址读取寄存器值到缓冲区
     * @param addr 寄存器地址
     * @param count 寄存器数量
     * @param buffer 目标缓冲区
     * @return 是否成功
     */
    bool readRegistersByAddress(u16 addr, u16 count, u8* buffer) const {
        u16 offset = 0;
        for (u16 i = 0; i < count; ++i) {
            u16 currentAddr = addr + i;
            if (!_readSingleRegister(currentAddr, buffer + offset)) {
                return false;
            }
            offset += 2;  // 每个寄存器2字节
        }
        return true;
    }

    /**
     * @brief 通过地址写入寄存器值
     * @param addr 寄存器地址
     * @param count 寄存器数量
     * @param buffer 源缓冲区
     * @return 是否成功
     */
    bool writeRegistersByAddress(u16 addr, u16 count, const u8* buffer) {
        u16 offset = 0;
        for (u16 i = 0; i < count; ++i) {
            u16 currentAddr = addr + i;
            if (!_writeSingleRegister(currentAddr, buffer + offset)) {
                return false;
            }
            offset += 2;  // 每个寄存器2字节
        }
        return true;
    }

    /**
     * @brief 通过地址读取寄存器到Slice
     */
    bool readRegistersByAddressToSlice(u16 addr, u16 count, Slice& slice,
                                       u16 sliceOffset = 0) const {
        u16 offset = sliceOffset;
        for (u16 i = 0; i < count; ++i) {
            u16 currentAddr = addr + i;
            if (!_readSingleRegisterToSlice(currentAddr, slice, offset)) {
                return false;
            }
            offset += 2;
        }
        return true;
    }

    /**
     * @brief 通过地址从Slice写入寄存器
     */
    bool writeRegistersByAddressFromSlice(u16 addr, u16 count, const Slice& slice,
                                          u16 sliceOffset = 0) {
        u16 offset = sliceOffset;
        for (u16 i = 0; i < count; ++i) {
            u16 currentAddr = addr + i;
            if (!_writeSingleRegisterFromSlice(currentAddr, slice, offset)) {
                return false;
            }
            offset += 2;
        }
        return true;
    }

    /**
     * @brief 通过类型获取寄存器（用于应用层直接访问）
     */
    template <typename RegDef>
    auto& get() {
        return std::get<RegisterItem<RegDef>>(_registers);
    }

    template <typename RegDef>
    const auto& get() const {
        return std::get<RegisterItem<RegDef>>(_registers);
    }

    /**
     * @brief 设置寄存器值（应用层API）
     */
    template <typename RegDef>
    void set(typename RegisterItem<RegDef>::ValueType value) {
        get<RegDef>().set(value);
    }

    /**
     * @brief 读取寄存器值（应用层API）
     */
    template <typename RegDef>
    auto read() const {
        return get<RegDef>().get();
    }

    /**
     * @brief 检查地址是否存在
     */
    bool hasAddress(u16 addr) const {
        return _findRegisterByAddress(addr) != nullptr;
    }

    /**
     * @brief 获取地址范围内的寄存器数量
     */
    u16 getRegisterCountInRange(u16 startAddr, u16 count) const {
        u16 validCount = 0;
        for (u16 i = 0; i < count; ++i) {
            if (hasAddress(startAddr + i)) {
                validCount++;
            }
        }
        return validCount;
    }

    /**
     * @brief 获取寄存器总数
     */
    static constexpr size_t size() {
        return sizeof...(RegDefs);
    }

   private:
    // 地址索引结构
    struct AddressIndexEntry {
        u16              address;
        u8               regIndex;  // 在tuple中的索引
        u8               subIndex;  // 对于32位寄存器，子索引(0或1)
        RegisterType     type;
        RegisterDataType dataType;
        RegisterAccess   access;
    };

    static constexpr size_t MAX_ADDRESS_ENTRIES = sizeof...(RegDefs) * 2;  // 32位寄存器占2个地址
    AddressIndexEntry       _addressIndex[MAX_ADDRESS_ENTRIES];
    size_t                  _addressIndexSize = 0;

    void _buildAddressIndex() {
        _addressIndexSize = 0;
        _buildAddressIndexForRegister<0, RegDefs...>();
    }

    template <size_t RegIndex, typename First, typename... Rest>
    void _buildAddressIndexForRegister() {
        // 添加当前寄存器的地址索引
        _addressIndex[_addressIndexSize++] = {First::Address,
                                              static_cast<u8>(RegIndex),
                                              0,  // 主索引
                                              First::Type,
                                              First::DataType,
                                              First::Access};

        // 如果是32位寄存器，添加第二个地址
        if constexpr (First::RegisterCount == 2) {
            _addressIndex[_addressIndexSize++] = {static_cast<u16>(First::Address + 1),
                                                  static_cast<u8>(RegIndex),
                                                  1,  // 子索引
                                                  First::Type,
                                                  First::DataType,
                                                  First::Access};
        }

        // 递归处理其余寄存器
        if constexpr (sizeof...(Rest) > 0) {
            _buildAddressIndexForRegister<RegIndex + 1, Rest...>();
        }
    }

    const AddressIndexEntry* _findRegisterByAddress(u16 addr) const {
        for (size_t i = 0; i < _addressIndexSize; ++i) {
            if (_addressIndex[i].address == addr) {
                return &_addressIndex[i];
            }
        }
        return nullptr;
    }

    bool _readSingleRegister(u16 addr, u8* buffer) const {
        return _readWriteRegisterByIndex<true>(addr, buffer);
    }

    bool _writeSingleRegister(u16 addr, const u8* buffer) {
        return _readWriteRegisterByIndex<false>(addr, const_cast<u8*>(buffer));
    }

    bool _readSingleRegisterToSlice(u16 addr, Slice& slice, u16 offset) const {
        u8 buffer[2];
        if (_readSingleRegister(addr, buffer)) {
            slice.setUint16(offset, (static_cast<u16>(buffer[0]) << 8) | buffer[1], Endian::kBig);
            return true;
        }
        return false;
    }

    bool _writeSingleRegisterFromSlice(u16 addr, const Slice& slice, u16 offset) {
        u16 value     = slice.getUint16(offset, Endian::kBig);
        u8  buffer[2] = {static_cast<u8>(value >> 8), static_cast<u8>(value & 0xFF)};
        return _writeSingleRegister(addr, buffer);
    }

    template <bool IsRead>
    bool _readWriteRegisterByIndex(u16 addr, u8* buffer) const {
        const auto* entry = _findRegisterByAddress(addr);
        if (!entry) return false;

        // 访问权限检查 - 从Master视角定义的权限
        if constexpr (IsRead) {
            // Master读请求 -> Slave输出数据 -> 检查Master是否可读
            if (entry->access == RegisterAccess::kWriteOnly) return false;
        } else {
            // Master写请求 -> Slave接收数据 -> 检查Master是否可写
            if (entry->access == RegisterAccess::kReadOnly) return false;
        }

        return _accessRegisterByEntry<IsRead>(*entry, buffer);
    }

    template <bool IsRead>
    bool _accessRegisterByEntry(const AddressIndexEntry& entry, u8* buffer) const {
        // 使用编译期分发到具体的寄存器类型
        return _dispatchRegisterAccess<IsRead, 0, RegDefs...>(entry, buffer);
    }

    template <bool IsRead, size_t RegIndex, typename First, typename... Rest>
    bool _dispatchRegisterAccess(const AddressIndexEntry& entry, u8* buffer) const {
        if (entry.regIndex == RegIndex) {
            const auto& regItem = std::get<RegIndex>(_registers);

            if constexpr (IsRead) {
                if constexpr (First::RegisterCount == 1) {
                    // 16位寄存器
                    auto value = regItem.value();
                    buffer[0]  = (value >> 8) & 0xFF;
                    buffer[1]  = value & 0xFF;
                } else {
                    // 32位寄存器，根据子索引返回高位或低位
                    auto value = regItem.value();
                    if (entry.subIndex == 0) {
                        // 高16位
                        u16 highWord = (value >> 16) & 0xFFFF;
                        buffer[0]    = (highWord >> 8) & 0xFF;
                        buffer[1]    = highWord & 0xFF;
                    } else {
                        // 低16位
                        u16 lowWord = value & 0xFFFF;
                        buffer[0]   = (lowWord >> 8) & 0xFF;
                        buffer[1]   = lowWord & 0xFF;
                    }
                }
            } else {
                if constexpr (First::RegisterCount == 1) {
                    // 16位寄存器
                    typename RegisterItem<First>::ValueType value =
                        (static_cast<u16>(buffer[0]) << 8) | buffer[1];
                    const_cast<RegisterItem<First>&>(regItem).value() = value;
                } else {
                    // 32位寄存器，需要组合或分解
                    u16  word         = (static_cast<u16>(buffer[0]) << 8) | buffer[1];
                    auto currentValue = regItem.value();
                    if (entry.subIndex == 0) {
                        // 更新高16位
                        currentValue = (static_cast<u32>(word) << 16) | (currentValue & 0xFFFF);
                    } else {
                        // 更新低16位
                        currentValue = (currentValue & 0xFFFF0000) | word;
                    }
                    const_cast<RegisterItem<First>&>(regItem).value() = currentValue;
                }
            }
            return true;
        }

        if constexpr (sizeof...(Rest) > 0) {
            return _dispatchRegisterAccess<IsRead, RegIndex + 1, Rest...>(entry, buffer);
        }
        return false;
    }

    template <typename RegDef>
    void _initRegister() {
        // 编译期验证寄存器地址不重复
        static_assert(checkNoDuplicateAddress<RegDefs...>(), "Duplicate register address detected");

        // 检查当前寄存器是否与其他寄存器有地址范围重叠
        static_assert(checkNoAddressOverlap<RegDefs...>(),
                      "Register address range overlaps with other registers");
    }

    // 重用MasterRegisterMap中的地址检查函数
    template <typename... AllRegDefs>
    static constexpr bool checkNoDuplicateAddress() {
        return checkNoDuplicateAddressImpl<AllRegDefs...>();
    }

    template <typename First, typename... Rest>
    static constexpr bool checkNoDuplicateAddressImpl() {
        if constexpr (sizeof...(Rest) == 0) {
            return true;
        } else {
            return !hasAddressConflict<First, Rest...>() && checkNoDuplicateAddressImpl<Rest...>();
        }
    }

    template <typename Check, typename... Others>
    static constexpr bool hasAddressConflict() {
        return ((Check::Address == Others::Address) || ...);
    }

    template <typename... AllRegDefs>
    static constexpr bool checkNoAddressOverlap() {
        return checkNoAddressOverlapImpl<AllRegDefs...>();
    }

    template <typename First, typename... Rest>
    static constexpr bool checkNoAddressOverlapImpl() {
        if constexpr (sizeof...(Rest) == 0) {
            return true;
        } else {
            return !hasAddressRangeOverlap<First, Rest...>() &&
                   checkNoAddressOverlapImpl<Rest...>();
        }
    }

    template <typename Check, typename... Others>
    static constexpr bool hasAddressRangeOverlap() {
        return (addressRangeOverlaps<Check, Others>() || ...);
    }

    template <typename Reg1, typename Reg2>
    static constexpr bool addressRangeOverlaps() {
        constexpr u16 reg1_start = Reg1::Address;
        constexpr u16 reg1_end   = Reg1::Address + Reg1::RegisterCount - 1;
        constexpr u16 reg2_start = Reg2::Address;
        constexpr u16 reg2_end   = Reg2::Address + Reg2::RegisterCount - 1;

        return !(reg1_end < reg2_start || reg2_end < reg1_start);
    }

    std::tuple<RegisterItem<RegDefs>...> _registers;
};

// ============================================================================
// 类型别名，用于向后兼容和便利
// ============================================================================

/**
 * @brief 默认RegisterMap为Master模式（向后兼容）
 */
template <typename... RegDefs>
using RegisterMap = MasterRegisterMap<RegDefs...>;

}  // namespace wibot::modbus

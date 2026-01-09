#pragma once

#include "../../base/type.hpp"
#include "register.hpp"
#include <tuple>
#include <type_traits>

namespace wibot {

// ============================================================================
// 寄存器镜像基类 - 不依赖模板参数的运行时操作
// ============================================================================

/**
 * @brief 寄存器镜像基类 - 提供不依赖模板参数的通用操作
 * 
 * 此基类包含所有只依赖运行时数据的方法，减少模板实例化开销
 */
class RegisterMirrorBase {
   public:
    /**
     * @brief 寄存器信息结构
     */
    struct RegisterInfo {
        u16        address;        // 寄存器地址
        u16        slotOffset;     // 在紧凑数组中的槽位偏移
        u16        registerCount;  // 寄存器数量
        SyncPolicy syncPolicy;     // 同步策略
        u32        syncInterval;   // 同步间隔(ms)
    };

   protected:
    virtual const u16*          getData() const          = 0;
    virtual u16*                getData()                = 0;
    virtual const u16*          getDirty() const         = 0;
    virtual u16*                getDirty()               = 0;
    virtual const RegisterInfo* getRegisterInfos() const = 0;
    virtual size_t              getRegisterCount() const = 0;

   public:
    virtual ~RegisterMirrorBase() = default;

    /**
     * @brief 检查寄存器是否脏(按索引)
     */
    bool isDirtyByIndex(u16 regIndex) const {
        size_t regCount = getRegisterCount();
        if (regIndex < regCount) {
            const u16* dirty = getDirty();
            return (dirty[regIndex / 16] & (1 << (regIndex % 16))) != 0;
        }
        return false;
    }

    /**
     * @brief 清除脏标记(按索引)
     */
    void clearDirtyByIndex(u16 regIndex) {
        size_t regCount = getRegisterCount();
        if (regIndex < regCount) {
            u16* dirty = getDirty();
            dirty[regIndex / 16] &= ~(1 << (regIndex % 16));
        }
    }

    /**
     * @brief 标记寄存器为脏(按索引)
     */
    void markDirtyByIndex(u16 regIndex) {
        size_t regCount = getRegisterCount();
        if (regIndex < regCount) {
            u16* dirty = getDirty();
            dirty[regIndex / 16] |= (1 << (regIndex % 16));
        }
    }

    /**
     * @brief 获取寄存器数据指针(按索引)
     */
    const u16* getRegisterDataByIndex(u16 regIndex) const {
        if (regIndex < getRegisterCount()) {
            const RegisterInfo* infos = getRegisterInfos();
            return getData() + infos[regIndex].slotOffset;
        }
        return nullptr;
    }

    u16* getRegisterDataByIndex(u16 regIndex) {
        if (regIndex < getRegisterCount()) {
            const RegisterInfo* infos = getRegisterInfos();
            return getData() + infos[regIndex].slotOffset;
        }
        return nullptr;
    }

    /**
     * @brief 获取原始数据数组指针
     */
    const u16* data() const {
        return getData();
    }

    u16* data() {
        return getData();
    }

    /**
     * @brief 根据Modbus地址获取数据指针(运行时查找)
     */
    const u16* dataAt(u16 addr) const {
        u16 slotIndex = findSlotByAddress(addr);
        if (slotIndex == 0xFFFF) return nullptr;
        return getData() + slotIndex;
    }

    u16* dataAt(u16 addr) {
        u16 slotIndex = findSlotByAddress(addr);
        if (slotIndex == 0xFFFF) return nullptr;
        return getData() + slotIndex;
    }

    /**
     * @brief 检查寄存器是否脏(按Modbus地址)
     */
    bool isDirtyByAddress(u16 addr) const {
        i16 regIndex = findRegisterIndexByAddress(addr);
        if (regIndex < 0) return false;
        const u16* dirty = getDirty();
        return (dirty[regIndex / 16] & (1 << (regIndex % 16))) != 0;
    }

   protected:
    /**
     * @brief 运行时查找:根据Modbus地址找到槽位索引
     */
    u16 findSlotByAddress(u16 addr) const {
        const RegisterInfo* infos    = getRegisterInfos();
        size_t              regCount = getRegisterCount();
        for (size_t i = 0; i < regCount; ++i) {
            if (addr >= infos[i].address && addr < infos[i].address + infos[i].registerCount) {
                return infos[i].slotOffset + (addr - infos[i].address);
            }
        }
        return 0xFFFF;
    }

    /**
     * @brief 运行时查找:根据Modbus地址找到寄存器索引
     */
    i16 findRegisterIndexByAddress(u16 addr) const {
        const RegisterInfo* infos    = getRegisterInfos();
        size_t              regCount = getRegisterCount();
        for (size_t i = 0; i < regCount; ++i) {
            if (addr >= infos[i].address && addr < infos[i].address + infos[i].registerCount) {
                return static_cast<i16>(i);
            }
        }
        return -1;
    }
};

// ============================================================================
// 寄存器镜像存储 - 使用模板元编程实现零开销抽象
// ============================================================================

/**
 * @brief 寄存器镜像 - 存储从机寄存器的本地副本
 * @tparam RegDefs 可变参数模板,包含所有寄存器定义
 * 
 * 特性:
 * 1. 编译期计算所有偏移量,零运行时开销
 * 2. 类型安全的访问接口
 * 3. 紧凑的内存布局 - 仅为实际定义的寄存器分配空间
 * 
 * 存储策略:
 * - 使用编译期计算的紧凑数组,每个寄存器定义占用连续的存储槽位
 * - 地址到槽位的映射在编译期完成,运行时零查找开销
 * - 例如: Reg<0x1000,2>, Reg<0x2000,1> 仅占用3个u16,而非0x2001个
 */
template <typename... RegDefs>
class RegisterMirror : public RegisterMirrorBase {
   private:
    // ========================================================================
    // 编译期元编程 - 类型推导和常量计算
    // ========================================================================

    struct Traits {
        // 检查类型是否在参数包中
        template <typename T, typename... Ts>
        struct Contains : std::false_type {};

        template <typename T, typename First, typename... Rest>
        struct Contains<T, First, Rest...>
            : std::conditional_t<std::is_same<T, First>::value, std::true_type,
                                 Contains<T, Rest...>> {};

        // 计算总共需要的存储槽位数量
        template <typename... Ts>
        struct TotalSlots;

        template <>
        struct TotalSlots<> {
            static constexpr u16 value = 0;
        };

        template <typename First, typename... Rest>
        struct TotalSlots<First, Rest...> {
            static constexpr u16 value = First::RegisterCount + TotalSlots<Rest...>::value;
        };

        // 计算特定寄存器在存储数组中的起始槽位索引
        template <typename Target, typename... Ts>
        struct SlotOffset;

        template <typename Target, typename First, typename... Rest>
        struct SlotOffset<Target, First, Rest...> {
            static constexpr u16 value =
                std::is_same<Target, First>::value
                    ? 0
                    : (First::RegisterCount + SlotOffset<Target, Rest...>::value);
        };

        template <typename Target>
        struct SlotOffset<Target> {
            static constexpr u16 value = 0;
        };

        // 获取寄存器索引(第几个寄存器定义)
        template <typename Target, u16 Index, typename... Ts>
        struct RegisterIndex;

        template <typename Target, u16 Index, typename First, typename... Rest>
        struct RegisterIndex<Target, Index, First, Rest...> {
            static constexpr u16 value = std::is_same<Target, First>::value
                                             ? Index
                                             : RegisterIndex<Target, Index + 1, Rest...>::value;
        };

        template <typename Target, u16 Index>
        struct RegisterIndex<Target, Index> {
            static constexpr u16 value = 0;
        };

        // 编译期常量
        static constexpr size_t kRegisterCount   = sizeof...(RegDefs);
        static constexpr size_t kTotalSlots      = TotalSlots<RegDefs...>::value;
        static constexpr size_t kDirtyBitmapSize = (kRegisterCount + 15) / 16;
    };

   private:
    // ========================================================================
    // 内部辅助元函数和数据结构
    // ========================================================================

    // 生成寄存器信息数组的辅助元函数
    template <u16 CurrentSlot, typename... Defs>
    struct BuildRegisterInfoArray;

    template <u16 CurrentSlot>
    struct BuildRegisterInfoArray<CurrentSlot> {
        static constexpr void fill(RegisterMirrorBase::RegisterInfo* arr, size_t idx) {
        }
    };

    template <u16 CurrentSlot, typename First, typename... Rest>
    struct BuildRegisterInfoArray<CurrentSlot, First, Rest...> {
        static constexpr void fill(RegisterMirrorBase::RegisterInfo* arr, size_t idx) {
            arr[idx] = {First::Address, CurrentSlot, First::RegisterCount, First::SyncPolicy_,
                        First::SyncInterval};
            BuildRegisterInfoArray<CurrentSlot + First::RegisterCount, Rest...>::fill(arr, idx + 1);
        }
    };

    // 编译期生成寄存器信息数组
    static constexpr auto makeRegisterInfoArray() {
        RegisterMirrorBase::RegisterInfo
            arr[Traits::kRegisterCount > 0 ? Traits::kRegisterCount : 1] = {};
        BuildRegisterInfoArray<0, RegDefs...>::fill(arr, 0);
        return arr;
    }

   private:
    // ========================================================================
    // 成员变量 - 必须在虚函数实现之前声明
    // ========================================================================

    // 紧凑存储数据 - 仅分配实际需要的空间
    u16 _data[Traits::kTotalSlots]{0};

    // 脏标记位图 - 每个寄存器定义一个位(而非每个地址)
    u16 _dirty[Traits::kDirtyBitmapSize > 0 ? Traits::kDirtyBitmapSize : 1]{0};

   protected:
    // ========================================================================
    // 基类虚函数实现
    // ========================================================================

    const u16* getData() const override {
        return _data;
    }
    u16* getData() override {
        return _data;
    }
    const u16* getDirty() const override {
        return _dirty;
    }
    u16* getDirty() override {
        return _dirty;
    }
    const RegisterMirrorBase::RegisterInfo* getRegisterInfos() const override {
        return kRegisterInfos;
    }
    size_t getRegisterCount() const override {
        return Traits::kRegisterCount;
    }

   public:
    /**
     * @brief 寄存器信息数组(编译期常量,存储在Flash/ROM)
     * 供同步管理器等组件使用
     */
    static constexpr RegisterMirrorBase::RegisterInfo
        kRegisterInfos[Traits::kRegisterCount > 0 ? Traits::kRegisterCount : 1] =
            makeRegisterInfoArray();

    // ========================================================================
    // 镜像使用者接口 - 类型安全的读写操作
    // ========================================================================

    /**
     * @brief 读取寄存器值
     * @tparam RegDef 寄存器定义类型
     * @return 寄存器值(根据数据类型自动转换)
     */
    template <typename RegDef>
    ALWAYS_INLINE RegisterValueType_t<RegDef> read() const {
        static_assert(Traits::template Contains<RegDef, RegDefs...>::value,
                      "Register not defined in this mirror");
        static_assert(RegDef::Access != RegisterAccess::kWriteOnly,
                      "Cannot read write-only register");

        constexpr u16 slotIndex = Traits::template SlotOffset<RegDef, RegDefs...>::value;
        return readValue<RegDef>(slotIndex);
    }

    /**
     * @brief 写入寄存器值(会自动标记为脏)
     * @tparam RegDef 寄存器定义类型
     * @param value 要写入的值
     */
    template <typename RegDef>
    ALWAYS_INLINE void write(RegisterValueType_t<RegDef> value) {
        static_assert(Traits::template Contains<RegDef, RegDefs...>::value,
                      "Register not defined in this mirror");
        static_assert(RegDef::Access != RegisterAccess::kReadOnly,
                      "Cannot write to read-only register");

        constexpr u16 slotIndex = Traits::template SlotOffset<RegDef, RegDefs...>::value;
        constexpr u16 regIndex  = Traits::template RegisterIndex<RegDef, 0, RegDefs...>::value;

        writeValue<RegDef>(slotIndex, value);
        markDirtyByIndex(regIndex);
    }

    /**
     * @brief 检查寄存器是否被修改(脏标记)
     * @tparam RegDef 寄存器定义类型
     * @return true=已修改,需要同步; false=未修改
     */
    template <typename RegDef>
    ALWAYS_INLINE bool isDirty() const {
        static_assert(Traits::template Contains<RegDef, RegDefs...>::value,
                      "Register not defined in this mirror");
        constexpr u16 regIndex = Traits::template RegisterIndex<RegDef, 0, RegDefs...>::value;
        return (_dirty[regIndex / 16] & (1 << (regIndex % 16))) != 0;
    }

    /**
     * @brief 手动标记寄存器为脏(需要同步)
     * @tparam RegDef 寄存器定义类型
     */
    template <typename RegDef>
    ALWAYS_INLINE void markDirty() {
        static_assert(Traits::template Contains<RegDef, RegDefs...>::value,
                      "Register not defined in this mirror");
        constexpr u16 regIndex = Traits::template RegisterIndex<RegDef, 0, RegDefs...>::value;
        _dirty[regIndex / 16] |= (1 << (regIndex % 16));
    }

    /**
     * @brief 清除寄存器的脏标记
     * @tparam RegDef 寄存器定义类型
     */
    template <typename RegDef>
    ALWAYS_INLINE void clearDirty() {
        static_assert(Traits::template Contains<RegDef, RegDefs...>::value,
                      "Register not defined in this mirror");
        constexpr u16 regIndex = Traits::template RegisterIndex<RegDef, 0, RegDefs...>::value;
        _dirty[regIndex / 16] &= ~(1 << (regIndex % 16));
    }

    /**
     * @brief 清空镜像数据并重置脏标记
     * 
     * 注意: 主机侧镜像仅负责缓存数据，寄存器默认值由从机固件负责初始化
     */
    void reset() {
        // 清空所有数据
        for (size_t i = 0; i < Traits::kTotalSlots; ++i) {
            _data[i] = 0;
        }

        // 清除脏标记
        for (size_t i = 0; i < Traits::kDirtyBitmapSize; ++i) {
            _dirty[i] = 0;
        }
    }

    /**
     * @brief 获取总槽位数(所有寄存器占用的u16数量)
     * @return 槽位总数
     */
    static constexpr size_t getTotalSlots() {
        return Traits::kTotalSlots;
    }

    /**
     * @brief 获取寄存器地址(编译期)
     * @tparam RegDef 寄存器定义类型
     * @return 寄存器地址
     */
    template <typename RegDef>
    static constexpr u16 getAddress() {
        return RegDef::Address;
    }

    // ========================================================================
    // 高级接口 - 按地址/原始数据访问
    // ========================================================================

    /**
     * @brief 获取特定寄存器的数据指针(编译期)
     * @tparam RegDef 寄存器定义类型
     * @return 数据指针
     */
    template <typename RegDef>
    ALWAYS_INLINE const u16* getRegisterData() const {
        static_assert(Traits::template Contains<RegDef, RegDefs...>::value,
                      "Register not defined in this mirror");
        constexpr u16 slotIndex = Traits::template SlotOffset<RegDef, RegDefs...>::value;
        return &_data[slotIndex];
    }

    template <typename RegDef>
    ALWAYS_INLINE u16* getRegisterData() {
        static_assert(Traits::template Contains<RegDef, RegDefs...>::value,
                      "Register not defined in this mirror");
        constexpr u16 slotIndex = Traits::template SlotOffset<RegDef, RegDefs...>::value;
        return &_data[slotIndex];
    }

    // ========================================================================
    // 内部实现方法
    // ========================================================================

    // 按 DataType 特化的底层读取实现(5种数据类型共享)
    template <RegisterDataType DType>
    ALWAYS_INLINE auto readValueByType(u16 slotIndex) const {
        if constexpr (DType == RegisterDataType::kUint16) {
            return _data[slotIndex];
        } else if constexpr (DType == RegisterDataType::kInt16) {
            return static_cast<i16>(_data[slotIndex]);
        } else if constexpr (DType == RegisterDataType::kUint32) {
            return (static_cast<u32>(_data[slotIndex]) << 16) | _data[slotIndex + 1];
        } else if constexpr (DType == RegisterDataType::kInt32) {
            u32 temp = (static_cast<u32>(_data[slotIndex]) << 16) | _data[slotIndex + 1];
            return static_cast<i32>(temp);
        } else if constexpr (DType == RegisterDataType::kFloat) {
            u32   temp = (static_cast<u32>(_data[slotIndex]) << 16) | _data[slotIndex + 1];
            float result;
            __builtin_memcpy(&result, &temp, sizeof(float));
            return result;
        }
    }

    // 按 DataType 特化的底层写入实现(5种数据类型共享)
    template <RegisterDataType DType, typename ValueType>
    ALWAYS_INLINE void writeValueByType(u16 slotIndex, ValueType value) {
        if constexpr (DType == RegisterDataType::kUint16) {
            _data[slotIndex] = value;
        } else if constexpr (DType == RegisterDataType::kInt16) {
            _data[slotIndex] = static_cast<u16>(value);
        } else if constexpr (DType == RegisterDataType::kUint32) {
            _data[slotIndex]     = static_cast<u16>(value >> 16);
            _data[slotIndex + 1] = static_cast<u16>(value & 0xFFFF);
        } else if constexpr (DType == RegisterDataType::kInt32) {
            u32 temp             = static_cast<u32>(value);
            _data[slotIndex]     = static_cast<u16>(temp >> 16);
            _data[slotIndex + 1] = static_cast<u16>(temp & 0xFFFF);
        } else if constexpr (DType == RegisterDataType::kFloat) {
            u32 temp;
            __builtin_memcpy(&temp, &value, sizeof(float));
            _data[slotIndex]     = static_cast<u16>(temp >> 16);
            _data[slotIndex + 1] = static_cast<u16>(temp & 0xFFFF);
        }
    }

    // RegDef版本的读取(转发到DataType版本,会被内联)
    template <typename RegDef>
    ALWAYS_INLINE RegisterValueType_t<RegDef> readValue(u16 slotIndex) const {
        return readValueByType<RegDef::DataType>(slotIndex);
    }

    // RegDef版本的写入(转发到DataType版本,会被内联)
    template <typename RegDef>
    ALWAYS_INLINE void writeValue(u16 slotIndex, RegisterValueType_t<RegDef> value) {
        writeValueByType<RegDef::DataType>(slotIndex, value);
    }
};

// ============================================================================
// 寄存器访问器 - 为特定从机提供类型安全的访问接口
// ============================================================================

/**
 * @brief 寄存器访问器 - 封装对寄存器镜像的访问
 * @tparam SlaveAddr 从机地址
 * @tparam RegDefs 寄存器定义列表
 * 
 * 使用方式:
 * ```cpp
 * using MySlaveRegs = RegisterAccessor<0x01, Reg1, Reg2, Reg3>;
 * MySlaveRegs accessor(mirror);
 * auto value = accessor.read<Reg1>();
 * accessor.write<Reg2>(123);
 * ```
 */
template <u8 SlaveAddr, typename... RegDefs>
class RegisterAccessor {
   private:
    RegisterMirror<RegDefs...>& _mirror;

   public:
    static constexpr u8 kSlaveAddress = SlaveAddr;

    explicit RegisterAccessor(RegisterMirror<RegDefs...>& mirror) : _mirror(mirror) {
    }

    /**
     * @brief 读取寄存器
     */
    template <typename RegDef>
    ALWAYS_INLINE RegisterValueType_t<RegDef> read() const {
        return _mirror.template read<RegDef>();
    }

    /**
     * @brief 写入寄存器
     */
    template <typename RegDef>
    ALWAYS_INLINE void write(RegisterValueType_t<RegDef> value) {
        _mirror.template write<RegDef>(value);
    }

    /**
     * @brief 获取底层镜像
     */
    ALWAYS_INLINE RegisterMirror<RegDefs...>& mirror() {
        return _mirror;
    }
    ALWAYS_INLINE const RegisterMirror<RegDefs...>& mirror() const {
        return _mirror;
    }
};

} // namespace wibot


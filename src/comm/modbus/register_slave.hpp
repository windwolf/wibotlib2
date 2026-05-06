#pragma once

#include "../../base/type.hpp"
#include "../../base/buffer.hpp"
#include "register.hpp"
#include "modbus.hpp"
#include <type_traits>

namespace wibot {

// ============================================================================
// 从机寄存器处理器基类
// ============================================================================

/**
 * @brief 从机寄存器处理器基类 - 不依赖模板参数的运行时操作
 */
class RegisterSlaveHandlerBase {
   public:
    /**
     * @brief 从机寄存器信息结构
     */
    struct RegisterInfo {
        u16            address;        // 寄存器地址
        u16            registerCount;  // 寄存器数量
        RegisterType   type;           // 寄存器类型
        RegisterAccess access;         // 访问权限
    };

   protected:
    virtual const RegisterInfo* getRegisterInfos() const = 0;
    virtual size_t              getRegisterCount() const = 0;

   public:
    virtual ~RegisterSlaveHandlerBase() = default;

    /**
     * @brief 根据地址查找寄存器索引
     * @param type 寄存器类型
     * @param addr 寄存器地址
     * @return 寄存器索引，未找到返回-1
     */
    i16 findRegisterIndex(RegisterType type, u16 addr) const {
        const RegisterInfo* infos    = getRegisterInfos();
        size_t              regCount = getRegisterCount();
        for (size_t i = 0; i < regCount; ++i) {
            if (infos[i].type == type && addr >= infos[i].address &&
                addr < infos[i].address + infos[i].registerCount) {
                return static_cast<i16>(i);
            }
        }
        return -1;
    }

    /**
     * @brief 检查寄存器访问权限
     * @param regIndex 寄存器索引
     * @param isRead true=读操作, false=写操作
     * @return true=允许访问
     */
    bool checkAccess(u16 regIndex, bool isRead) const {
        const RegisterInfo* infos = getRegisterInfos();
        if (regIndex >= getRegisterCount()) return false;

        RegisterAccess access = infos[regIndex].access;
        if (isRead) {
            return access == RegisterAccess::kReadOnly || access == RegisterAccess::kReadWrite;
        } else {
            return access == RegisterAccess::kWriteOnly || access == RegisterAccess::kReadWrite;
        }
    }
};

// ============================================================================
// 从机寄存器处理器 - 基于回调的分散式管理
// ============================================================================

/**
 * @brief 从机寄存器处理器
 * @tparam RegDefs 寄存器定义列表（使用 RegisterSlaveDef）
 * 
 * 特性:
 * 1. 不集中存储寄存器数据（数据分散在各处）
 * 2. 通过回调机制处理读写请求
 * 3. 自动处理权限检查和地址映射
 * 4. 支持初始化时使用默认值
 * 
 * 使用方式:
 * ```cpp
 * // 1. 定义寄存器（从机端，包含默认值）
 * using RegBase1 = RegisterBaseDef<0x1000, RegisterType::kHoldingRegister, 
 *                                   RegisterDataType::kUint16, RegisterAccess::kReadWrite>;
 * using Reg1 = RegisterSlaveDef<RegBase1, 100>;  // 默认值100
 * 
 * // 2. 创建处理器
 * RegisterSlaveHandler<Reg1, Reg2, Reg3> handler;
 * 
 * // 3. 设置读回调
 * handler.setReadCallback([](u16 regIndex, u16 addr, u16 offset, u16* data) -> bool {
 *     if (regIndex == 0) {  // Reg1
 *         *data = getActualValue();  // 从实际位置读取
 *         return true;
 *     }
 *     return false;
 * });
 * 
 * // 4. 设置写回调
 * handler.setWriteCallback([](u16 regIndex, u16 addr, u16 offset, u16 data) -> bool {
 *     if (regIndex == 0) {  // Reg1
 *         setActualValue(data);  // 写入实际位置
 *         return true;
 *     }
 *     return false;
 * });
 * 
 * // 5. 创建从机并绑定处理器
 * ModbusSlave slave(uart, 0x01, handler);
 * 
 * // 6. 在主循环中处理请求
 * slave.process();
 * ```
 */
template <typename... RegDefs>
class RegisterSlaveHandler : public RegisterSlaveHandlerBase, public IModbusSlaveHandler {
   public:
    /**
     * @brief 读回调函数类型
     * @param regIndex 寄存器索引（第几个寄存器定义）
     * @param addr 实际访问的寄存器地址
     * @param offset 相对于寄存器起始地址的偏移
     * @param data 输出数据指针（单个u16）
     * @return true=成功读取, false=读取失败
     */
    using ReadCallback = bool (*)(u16 regIndex, u16 addr, u16 offset, u16* data);

    /**
     * @brief 写回调函数类型
     * @param regIndex 寄存器索引（第几个寄存器定义）
     * @param addr 实际访问的寄存器地址
     * @param offset 相对于寄存器起始地址的偏移
     * @param data 输入数据（单个u16）
     * @return true=成功写入, false=写入失败
     */
    using WriteCallback = bool (*)(u16 regIndex, u16 addr, u16 offset, u16 data);

   private:
    // ========================================================================
    // 编译期元编程
    // ========================================================================

    // 生成寄存器信息数组的辅助元函数
    template <typename... Defs>
    struct BuildRegisterInfoArray;

    template <>
    struct BuildRegisterInfoArray<> {
        static constexpr void fill(RegisterSlaveHandlerBase::RegisterInfo* arr, size_t idx) {
        }
    };

    template <typename First, typename... Rest>
    struct BuildRegisterInfoArray<First, Rest...> {
        static constexpr void fill(RegisterSlaveHandlerBase::RegisterInfo* arr, size_t idx) {
            arr[idx] = {First::Address, First::RegisterCount, First::Type, First::Access};
            BuildRegisterInfoArray<Rest...>::fill(arr, idx + 1);
        }
    };

    // 编译期生成寄存器信息数组
    static constexpr auto makeRegisterInfoArray() {
        RegisterSlaveHandlerBase::RegisterInfo
            arr[sizeof...(RegDefs) > 0 ? sizeof...(RegDefs) : 1] = {};
        BuildRegisterInfoArray<RegDefs...>::fill(arr, 0);
        return arr;
    }

    // 编译期常量
    static constexpr size_t kRegisterCount = sizeof...(RegDefs);

   public:
    /**
     * @brief 寄存器信息数组（编译期常量）
     */
    static constexpr RegisterSlaveHandlerBase::RegisterInfo
        kRegisterInfos[kRegisterCount > 0 ? kRegisterCount : 1] = makeRegisterInfoArray();

   protected:
    const RegisterSlaveHandlerBase::RegisterInfo* getRegisterInfos() const override {
        return kRegisterInfos;
    }

    size_t getRegisterCount() const override {
        return kRegisterCount;
    }

   public:
    RegisterSlaveHandler() : _readCallback(nullptr), _writeCallback(nullptr) {
    }

    /**
     * @brief 设置读回调函数
     */
    void setReadCallback(ReadCallback callback) {
        _readCallback = callback;
    }

    /**
     * @brief 设置写回调函数
     */
    void setWriteCallback(WriteCallback callback) {
        _writeCallback = callback;
    }

    // ========================================================================
    // IModbusSlaveHandler 接口实现
    // ========================================================================

    /**
     * @brief 读取寄存器
     */
    Result onRead(RegisterType type, u16 addr, u16 count, Slice& data) override {
        // 查找起始寄存器
        i16 regIndex = findRegisterIndex(type, addr);
        if (regIndex < 0) {
            return Result::kInvalidParameter;
        }

        // 检查读权限
        if (!checkAccess(regIndex, true)) {
            return Result::kNotSupport;
        }

        const auto& regInfo = kRegisterInfos[regIndex];

        // 检查是否跨寄存器定义
        if (addr + count > regInfo.address + regInfo.registerCount) {
            return Result::kInvalidParameter;
        }

        // 确保输出缓冲区足够大
        if (data.size < count * 2) {
            return Result::kNoResource;
        }

        // 通过回调读取数据
        if (!_readCallback) {
            return Result::kNotSupport;
        }

        u16* outData = reinterpret_cast<u16*>(data.data);
        for (u16 i = 0; i < count; ++i) {
            u16 currentAddr   = addr + i;
            u16 offset        = currentAddr - regInfo.address;
            u16 currentRegIdx = regIndex;

            // 检查是否需要切换到下一个寄存器定义
            if (currentAddr >= regInfo.address + regInfo.registerCount) {
                currentRegIdx = findRegisterIndex(type, currentAddr);
                if (currentRegIdx < 0 || !checkAccess(currentRegIdx, true)) {
                    return Result::kInvalidParameter;
                }
            }

            if (!_readCallback(currentRegIdx, currentAddr, offset, &outData[i])) {
                return Result::kError;
            }
        }

        return Result::kOk;
    }

    /**
     * @brief 写入寄存器
     */
    Result onWrite(RegisterType type, u16 addr, u16 count, const Slice& data) override {
        // 查找起始寄存器
        i16 regIndex = findRegisterIndex(type, addr);
        if (regIndex < 0) {
            return Result::kInvalidParameter;
        }

        // 检查写权限
        if (!checkAccess(regIndex, false)) {
            return Result::kNotSupport;
        }

        const auto& regInfo = kRegisterInfos[regIndex];

        // 检查是否跨寄存器定义
        if (addr + count > regInfo.address + regInfo.registerCount) {
            return Result::kInvalidParameter;
        }

        // 确保输入缓冲区足够大
        if (data.size < count * 2) {
            return Result::kNoResource;
        }

        // 通过回调写入数据
        if (!_writeCallback) {
            return Result::kNotSupport;
        }

        const u16* inData = reinterpret_cast<const u16*>(data.data);
        for (u16 i = 0; i < count; ++i) {
            u16 currentAddr   = addr + i;
            u16 offset        = currentAddr - regInfo.address;
            u16 currentRegIdx = regIndex;

            // 检查是否需要切换到下一个寄存器定义
            if (currentAddr >= regInfo.address + regInfo.registerCount) {
                currentRegIdx = findRegisterIndex(type, currentAddr);
                if (currentRegIdx < 0 || !checkAccess(currentRegIdx, false)) {
                    return Result::kInvalidParameter;
                }
            }

            if (!_writeCallback(currentRegIdx, currentAddr, offset, inData[i])) {
                return Result::kError;
            }
        }

        return Result::kOk;
    }

    // ========================================================================
    // 工具方法
    // ========================================================================

    /**
     * @brief 获取寄存器的默认值（编译期）
     * @tparam RegDef 寄存器定义类型
     * @return 默认值
     */
    template <typename RegDef>
    static constexpr u32 getDefaultValue() {
        return RegDef::DefaultValue;
    }

    /**
     * @brief 获取寄存器数量
     */
    static constexpr size_t getRegCount() {
        return kRegisterCount;
    }

   private:
    ReadCallback  _readCallback;   // 读回调函数
    WriteCallback _writeCallback;  // 写回调函数
};

// ============================================================================
// 带默认值存储的从机寄存器处理器
// ============================================================================

/**
 * @brief 带默认值存储的从机寄存器处理器
 * @tparam RegDefs 寄存器定义列表（使用 RegisterSlaveDef）
 * 
 * 与 RegisterSlaveHandler 的区别:
 * - 内部维护一份默认值存储（使用寄存器定义中的默认值初始化）
 * - 如果回调未处理，则使用/更新默认值存储
 * - 适用于部分寄存器需要默认存储的场景
 * 
 * 使用方式:
 * ```cpp
 * RegisterSlaveHandlerWithDefaults<Reg1, Reg2, Reg3> handler;
 * 
 * // 设置回调（可选，仅处理需要特殊处理的寄存器）
 * handler.setReadCallback([](u16 regIndex, u16 addr, u16 offset, u16* data) -> bool {
 *     if (regIndex == 0) {  // Reg1 需要从硬件读取
 *         *data = readFromHardware();
 *         return true;
 *     }
 *     return false;  // 其他寄存器使用默认存储
 * });
 * 
 * // 未处理的寄存器自动使用默认值存储
 * ```
 */
template <typename... RegDefs>
class RegisterSlaveHandlerWithDefaults : public RegisterSlaveHandler<RegDefs...> {
   private:
    using Base = RegisterSlaveHandler<RegDefs...>;

    // 计算总存储槽位数
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

    // 计算寄存器在存储数组中的槽位偏移
    template <u16 CurrentSlot, typename... Defs>
    struct BuildSlotOffsets;

    template <u16 CurrentSlot>
    struct BuildSlotOffsets<CurrentSlot> {
        static constexpr void fill(u16* arr, size_t idx) {
        }
    };

    template <u16 CurrentSlot, typename First, typename... Rest>
    struct BuildSlotOffsets<CurrentSlot, First, Rest...> {
        static constexpr void fill(u16* arr, size_t idx) {
            arr[idx] = CurrentSlot;
            BuildSlotOffsets<CurrentSlot + First::RegisterCount, Rest...>::fill(arr, idx + 1);
        }
    };

    // 初始化默认值
    template <u16 SlotOffset, typename... Defs>
    struct InitDefaults;

    template <u16 SlotOffset>
    struct InitDefaults<SlotOffset> {
        static void init(u16* data) {
        }
    };

    template <u16 SlotOffset, typename First, typename... Rest>
    struct InitDefaults<SlotOffset, First, Rest...> {
        static void init(u16* data) {
            // 初始化当前寄存器的默认值
            if constexpr (First::RegisterCount == 1) {
                data[SlotOffset] = static_cast<u16>(First::DefaultValue);
            } else {
                // 32位类型，拆分为两个u16
                data[SlotOffset]     = static_cast<u16>(First::DefaultValue >> 16);
                data[SlotOffset + 1] = static_cast<u16>(First::DefaultValue & 0xFFFF);
            }
            InitDefaults<SlotOffset + First::RegisterCount, Rest...>::init(data);
        }
    };

    static constexpr size_t kTotalSlots = TotalSlots<RegDefs...>::value;

    // 槽位偏移数组
    u16 _slotOffsets[sizeof...(RegDefs) > 0 ? sizeof...(RegDefs) : 1];

    // 默认值存储
    u16 _defaultStorage[kTotalSlots > 0 ? kTotalSlots : 1];

   public:
    RegisterSlaveHandlerWithDefaults() {
        // 初始化槽位偏移
        BuildSlotOffsets<0, RegDefs...>::fill(_slotOffsets, 0);

        // 初始化默认值
        InitDefaults<0, RegDefs...>::init(_defaultStorage);

        // 设置包装后的回调
        Base::setReadCallback(
            [](u16 regIndex, u16 addr, u16 offset, u16* data) -> bool { return false; });
        Base::setWriteCallback(
            [](u16 regIndex, u16 addr, u16 offset, u16 data) -> bool { return false; });
    }

    /**
     * @brief 设置用户读回调（如果返回false，则使用默认存储）
     */
    void setUserReadCallback(typename Base::ReadCallback callback) {
        _userReadCallback = callback;
    }

    /**
     * @brief 设置用户写回调（如果返回false，则写入默认存储）
     */
    void setUserWriteCallback(typename Base::WriteCallback callback) {
        _userWriteCallback = callback;
    }

    /**
     * @brief 读取寄存器（优先调用用户回调，失败则使用默认存储）
     */
    Result onRead(RegisterType type, u16 addr, u16 count, Slice& data) override {
        // 查找起始寄存器
        i16 regIndex = this->findRegisterIndex(type, addr);
        if (regIndex < 0) {
            return Result::kInvalidParameter;
        }

        if (!this->checkAccess(regIndex, true)) {
            return Result::kNotSupport;
        }

        const auto& regInfo = Base::kRegisterInfos[regIndex];
        if (addr + count > regInfo.address + regInfo.registerCount) {
            return Result::kInvalidParameter;
        }

        if (data.size < count * 2) {
            return Result::kNoResource;
        }

        u16* outData = reinterpret_cast<u16*>(data.data);
        for (u16 i = 0; i < count; ++i) {
            u16 currentAddr = addr + i;
            u16 offset      = currentAddr - regInfo.address;

            // 先尝试用户回调
            bool handled = false;
            if (_userReadCallback) {
                handled = _userReadCallback(regIndex, currentAddr, offset, &outData[i]);
            }

            // 如果用户未处理，使用默认存储
            if (!handled) {
                u16 slotIndex = _slotOffsets[regIndex] + offset;
                outData[i]    = _defaultStorage[slotIndex];
            }
        }

        return Result::kOk;
    }

    /**
     * @brief 写入寄存器（优先调用用户回调，失败则写入默认存储）
     */
    Result onWrite(RegisterType type, u16 addr, u16 count, const Slice& data) override {
        // 查找起始寄存器
        i16 regIndex = this->findRegisterIndex(type, addr);
        if (regIndex < 0) {
            return Result::kInvalidParameter;
        }

        if (!this->checkAccess(regIndex, false)) {
            return Result::kNotSupport;
        }

        const auto& regInfo = Base::kRegisterInfos[regIndex];
        if (addr + count > regInfo.address + regInfo.registerCount) {
            return Result::kInvalidParameter;
        }

        if (data.size < count * 2) {
            return Result::kNoResource;
        }

        const u16* inData = reinterpret_cast<const u16*>(data.data);
        for (u16 i = 0; i < count; ++i) {
            u16 currentAddr = addr + i;
            u16 offset      = currentAddr - regInfo.address;

            // 先尝试用户回调
            bool handled = false;
            if (_userWriteCallback) {
                handled = _userWriteCallback(regIndex, currentAddr, offset, inData[i]);
            }

            // 如果用户未处理，写入默认存储
            if (!handled) {
                u16 slotIndex              = _slotOffsets[regIndex] + offset;
                _defaultStorage[slotIndex] = inData[i];
            }
        }

        return Result::kOk;
    }

    /**
     * @brief 直接访问默认存储（用于初始化或调试）
     */
    u16* getDefaultStorage() {
        return _defaultStorage;
    }

    const u16* getDefaultStorage() const {
        return _defaultStorage;
    }

   private:
    typename Base::ReadCallback  _userReadCallback  = nullptr;
    typename Base::WriteCallback _userWriteCallback = nullptr;
};

} // namespace wibot


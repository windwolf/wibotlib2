#pragma once

#include "../../base/type.hpp"
#include "../../base/buffer.hpp"
#include "modbus.hpp"
#include <tuple>
#include <type_traits>

namespace wibot::comm {

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
 * @brief 同步策略枚举
 */
enum class SyncPolicy : u8 {
    kNone,       // 不自动同步(手动调用)
    kReadOnly,   // 只读同步(定期从从机读取)
    kWriteOnly,  // 只写同步(本地修改后立即写入从机)
    kReadWrite   // 双向同步(定期读取+修改后写入)
};

// ============================================================================
// 三层寄存器定义体系
// ============================================================================

/**
 * @brief 寄存器基础定义 - 主从机共享的寄存器属性
 * @tparam ADDR 寄存器地址
 * @tparam TYPE 寄存器类型
 * @tparam DATA_TYPE 数据类型
 * @tparam ACCESS 访问权限
 */
template <u16 ADDR, RegisterType TYPE = RegisterType::kHoldingRegister,
          RegisterDataType DATA_TYPE = RegisterDataType::kUint16,
          RegisterAccess   ACCESS    = RegisterAccess::kReadWrite>
struct RegisterBaseDef {
    static constexpr u16              Address  = ADDR;
    static constexpr RegisterType     Type     = TYPE;
    static constexpr RegisterDataType DataType = DATA_TYPE;
    static constexpr RegisterAccess   Access   = ACCESS;

    // 根据数据类型确定占用的寄存器数量
    static constexpr u16 RegisterCount =
        (DATA_TYPE == RegisterDataType::kUint16 || DATA_TYPE == RegisterDataType::kInt16) ? 1 : 2;
};

/**
 * @brief 寄存器主机端定义 - 包含主机关心的同步策略
 * @tparam BaseDef 基础寄存器定义
 * @tparam SYNC_POLICY 同步策略
 * @tparam SYNC_INTERVAL 同步间隔(ms),0表示不定期同步
 */
template <typename BaseDef, SyncPolicy SYNC_POLICY = SyncPolicy::kNone, u32 SYNC_INTERVAL = 0>
struct RegisterMasterDef : BaseDef {
    static constexpr SyncPolicy SyncPolicy_  = SYNC_POLICY;
    static constexpr u32        SyncInterval = SYNC_INTERVAL;
};

/**
 * @brief 寄存器从机端定义 - 包含从机关心的默认值
 * @tparam BaseDef 基础寄存器定义
 * @tparam DEFAULT 默认值
 */
template <typename BaseDef, u32 DEFAULT = 0>
struct RegisterSlaveDef : BaseDef {
    static constexpr u32 DefaultValue = DEFAULT;
};

/**
 * @brief 寄存器完整定义 - 包含所有属性(向后兼容)
 * @tparam ADDR 寄存器地址
 * @tparam TYPE 寄存器类型
 * @tparam DATA_TYPE 数据类型
 * @tparam ACCESS 访问权限
 * @tparam DEFAULT 默认值
 * @tparam SYNC_POLICY 同步策略
 * @tparam SYNC_INTERVAL 同步间隔(ms),0表示不定期同步
 */
template <u16 ADDR, RegisterType TYPE = RegisterType::kHoldingRegister,
          RegisterDataType DATA_TYPE = RegisterDataType::kUint16,
          RegisterAccess ACCESS = RegisterAccess::kReadWrite, u32 DEFAULT = 0,
          SyncPolicy SYNC_POLICY = SyncPolicy::kNone, u32 SYNC_INTERVAL = 0>
struct RegisterDef {
    static constexpr u16              Address      = ADDR;
    static constexpr RegisterType     Type         = TYPE;
    static constexpr RegisterDataType DataType     = DATA_TYPE;
    static constexpr RegisterAccess   Access       = ACCESS;
    static constexpr u32              DefaultValue = DEFAULT;
    static constexpr SyncPolicy       SyncPolicy_  = SYNC_POLICY;
    static constexpr u32              SyncInterval = SYNC_INTERVAL;

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

}  // namespace wibot::modbus

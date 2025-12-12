#pragma once

#include "../../base/type.hpp"
#include "../../base/buffer.hpp"
#include "register_mirror.hpp"
#include "modbus.hpp"

namespace wibot::modbus {

// ============================================================================
// 寄存器同步状态追踪
// ============================================================================

/**
 * @brief 单个寄存器的同步状态
 */
struct RegisterSyncState {
    u32  lastSyncTime;  // 上次同步时间戳(ms)
    bool needSync;      // 是否需要同步
};

// ============================================================================
// 寄存器同步管理器 - 非模板化实现
// ============================================================================

/**
 * @brief 寄存器同步管理器(非模板,运行时配置)
 * 
 * 核心优化:
 * - 完全消除模板,所有实例共享同一份代码
 * - 直接使用 RegisterMirror::kRegisterInfos (包含完整信息)
 * - 构造时保存 mirror 引用,避免后续调用反复传入
 * - 代码尺寸: 固定约400字节,与寄存器数量无关
 * 
 * 使用方式:
 * ```cpp
 * // 1. 定义寄存器镜像
 * RegisterMirror<Reg1, Reg2, Reg3> mirror;
 * 
 * // 2. 创建同步管理器(传入 mirror 引用)
 * RegisterSyncManager syncMgr(modbus, 0x01, mirror);
 * 
 * // 3. 周期调用(无需再传 mirror)
 * syncMgr.tick(millis());
 * ```
 */
class RegisterSyncManager {
   public:
    // ========================================================================
    // 构造/析构
    // ========================================================================

    /**
     * @brief 构造函数
     * @tparam RegDefs 寄存器定义列表
     * @param modbus Modbus主机实例
     * @param slaveAddr 从机地址
     * @param mirror 寄存器镜像引用
     */
    template <typename... RegDefs>
    RegisterSyncManager(ModbusMaster& modbus, u8 slaveAddr, RegisterMirror<RegDefs...>& mirror)
        : _modbus(modbus),
          _slaveAddr(slaveAddr),
          _registerInfos(mirror.kRegisterInfos),
          _registerCount(mirror.getRegisterCount()),
          _syncStates(new RegisterSyncState[mirror.getRegisterCount()]{}),
          _mirror(new MirrorAdapter<RegDefs...>(mirror)),
          _ownsAdapter(true) {
    }

    ~RegisterSyncManager() {
        delete[] _syncStates;
        if (_ownsAdapter) {
            delete _mirror;
        }
    }

    // 禁止拷贝和赋值
    RegisterSyncManager(const RegisterSyncManager&)            = delete;
    RegisterSyncManager& operator=(const RegisterSyncManager&) = delete;

    // ========================================================================
    // 自动同步接口
    // ========================================================================

    /**
     * @brief 周期性调用此函数执行自动同步
     * @param currentTime 当前时间戳(ms)
     * @return 本次同步的寄存器数量
     * 
     * 根据每个寄存器的同步策略和间隔,自动执行读取或写入同步
     */
    u8 tick(u32 currentTime) {
        u8 syncCount = 0;

        for (size_t i = 0; i < _registerCount; ++i) {
            const auto&        regInfo = _registerInfos[i];
            RegisterSyncState& state   = _syncStates[i];

            // 读同步
            if (shouldSyncRead(regInfo, currentTime, state)) {
                if (doSyncRead(regInfo, i)) {
                    updateSyncState(state, currentTime, true);
                    syncCount++;
                }
            }

            // 写同步
            if (shouldSyncWrite(regInfo) && _mirror->isDirtyByIndex(i)) {
                if (doSyncWrite(regInfo, i)) {
                    syncCount++;
                }
            }
        }

        return syncCount;
    }

    // ========================================================================
    // 手动同步接口
    // ========================================================================

    /**
     * @brief 手动同步单个寄存器(读取)
     * @param index 寄存器索引(0-based)
     * @return 同步结果
     */
    Result syncReadByIndex(size_t index) {
        if (index >= _registerCount) return Result::kError;
        return doSyncRead(_registerInfos[index], index) ? Result::kOk : Result::kError;
    }

    /**
     * @brief 手动写回单个寄存器(如果脏)
     * @param index 寄存器索引(0-based)
     * @return 写入结果
     */
    Result syncWriteByIndex(size_t index) {
        if (index >= _registerCount) return Result::kError;
        if (!_mirror->isDirtyByIndex(index)) return Result::kOk;
        return doSyncWrite(_registerInfos[index], index) ? Result::kOk : Result::kError;
    }

    /**
     * @brief 强制标记某个寄存器需要同步
     * @param index 寄存器索引(0-based)
     * 
     * 用于强制在下次 tick() 时读取该寄存器,无论是否到达同步间隔
     */
    void markDirtyByIndex(size_t index) {
        if (index < _registerCount) {
            _syncStates[index].needSync = true;
        }
    }

   private:
    // ========================================================================
    // Mirror 接口抽象(类型擦除)
    // ========================================================================

    /**
     * @brief Mirror 接口 - 用于类型擦除
     * 定义同步管理器需要的 mirror 操作
     */
    struct IMirror {
        virtual ~IMirror()                                            = default;
        virtual const u16* getRegisterDataByIndex(u16 regIndex) const = 0;
        virtual u16*       getRegisterDataByIndex(u16 regIndex)       = 0;
        virtual bool       isDirtyByIndex(u16 regIndex) const         = 0;
        virtual void       clearDirtyByIndex(u16 regIndex)            = 0;
    };

    /**
     * @brief Mirror 适配器 - 将模板化的 RegisterMirror 适配到 IMirror 接口
     */
    template <typename... RegDefs>
    class MirrorAdapter : public IMirror {
       public:
        explicit MirrorAdapter(RegisterMirror<RegDefs...>& mirror) : _mirror(mirror) {
        }

        const u16* getRegisterDataByIndex(u16 regIndex) const override {
            return _mirror.getRegisterDataByIndex(regIndex);
        }

        u16* getRegisterDataByIndex(u16 regIndex) override {
            return _mirror.getRegisterDataByIndex(regIndex);
        }

        bool isDirtyByIndex(u16 regIndex) const override {
            return _mirror.isDirtyByIndex(regIndex);
        }

        void clearDirtyByIndex(u16 regIndex) override {
            _mirror.clearDirtyByIndex(regIndex);
        }

       private:
        RegisterMirror<RegDefs...>& _mirror;
    };

    // ========================================================================
    // 内部实现方法
    // ========================================================================

    /**
     * @brief 判断是否需要执行读同步
     */
    bool shouldSyncRead(const RegisterMirrorBase::RegisterInfo& regInfo, u32 currentTime,
                        RegisterSyncState& state) const {
        // 只处理配置了读策略的寄存器
        if (regInfo.syncPolicy == SyncPolicy::kNone ||
            regInfo.syncPolicy == SyncPolicy::kWriteOnly) {
            return false;
        }

        // 检查是否到达同步间隔
        if (regInfo.syncInterval > 0) {
            if ((currentTime - state.lastSyncTime) < regInfo.syncInterval && !state.needSync) {
                return false;
            }
        }

        return true;
    }

    /**
     * @brief 判断是否需要执行写同步
     */
    bool shouldSyncWrite(const RegisterMirrorBase::RegisterInfo& regInfo) const {
        return regInfo.syncPolicy != SyncPolicy::kNone &&
               regInfo.syncPolicy != SyncPolicy::kReadOnly;
    }

    /**
     * @brief 更新同步状态
     */
    void updateSyncState(RegisterSyncState& state, u32 currentTime, bool success) {
        if (success) {
            state.lastSyncTime = currentTime;
            state.needSync     = false;
        }
    }

    /**
     * @brief 执行读取同步 - 从从机读取数据到镜像
     */
    bool doSyncRead(const RegisterMirrorBase::RegisterInfo& regInfo, size_t index) {
        const u16* data = _mirror->getRegisterDataByIndex(index);
        if (!data) return false;

        Slice  slice{reinterpret_cast<u8*>(const_cast<u16*>(data)),
                    static_cast<u16>(regInfo.registerCount * 2)};
        Result result = _modbus.read(RegisterType::kHoldingRegister, _slaveAddr, regInfo.address,
                                     regInfo.registerCount, slice);

        if (result.isOk()) {
            _mirror->clearDirtyByIndex(index);
            return true;
        }
        return false;
    }

    /**
     * @brief 执行写入同步 - 将镜像数据写入从机
     */
    bool doSyncWrite(const RegisterMirrorBase::RegisterInfo& regInfo, size_t index) {
        const u16* data = _mirror->getRegisterDataByIndex(index);
        if (!data) return false;

        Slice  slice{reinterpret_cast<u8*>(const_cast<u16*>(data)),
                    static_cast<u16>(regInfo.registerCount * 2)};
        Result result = _modbus.write(RegisterType::kHoldingRegister, _slaveAddr, regInfo.address,
                                      regInfo.registerCount, slice);

        if (result.isOk()) {
            _mirror->clearDirtyByIndex(index);
            return true;
        }
        return false;
    }

    // ========================================================================
    // 成员变量
    // ========================================================================

    ModbusMaster& _modbus;     // Modbus主机引用
    u8            _slaveAddr;  // 从机地址

    // 寄存器完整信息(地址、数量、同步策略等) - 来自 RegisterMirror::kRegisterInfos
    const RegisterMirrorBase::RegisterInfo* _registerInfos;
    size_t                                  _registerCount;  // 寄存器数量

    // 每个寄存器的同步状态(运行时,存储在RAM)
    RegisterSyncState* _syncStates;

    // Mirror 接口指针(用于访问非模板接口)
    IMirror* _mirror;
    bool     _ownsAdapter;  // 是否拥有 adapter 的所有权
};

}  // namespace wibot::modbus

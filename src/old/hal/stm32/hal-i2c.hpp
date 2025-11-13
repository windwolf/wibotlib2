#pragma once

//
// Created by zhouj on 2023/10/6.
//

#include "bus.hpp"

namespace wibot {

#ifdef HAL_I2C_MODULE_ENABLED
class HardI2cMaster : public I2cMasterBus, private PeripheralBase, private Initializable {
   public:
    explicit HardI2cMaster(I2C_HandleTypeDef* handle);
    ~HardI2cMaster();

   public:
    Result setBaseConfig(I2cMasterBaseConfig& config) override;
    Result setTransitionConfig(I2cMasterTransitionConfig& config) override;
    Result readReg(u16 regAddr, void* data, u32 size, WaitHandler& waitHandler) override;
    Result writeReg(u16 regAddr, void* data, u32 size, WaitHandler& waitHandler) override;
    Result read(void* data, u32 size, WaitHandler& waitHandler) override;
    Result write(void* data, u32 size, WaitHandler& waitHandler) override;

   private:
    void _init() override;

    static void onReadCplt(I2C_HandleTypeDef* instance);
    static void onWriteCplt(I2C_HandleTypeDef* instance);
    static void onError(I2C_HandleTypeDef* instance);

   private:
    I2C_HandleTypeDef*        _handle;
    I2cMasterTransitionConfig _transitionConfig;
    u32                       _i2cDelay;

    WaitTrigger _waitTrigger;
};
#endif  // HAL_I2C_MODULE_ENABLED

}  // namespace wibot

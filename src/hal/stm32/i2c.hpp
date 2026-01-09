#pragma once

#include "bus.hpp"
#include "chip.hpp"
#include "peripheral.hpp"

namespace wibot::hal {

#ifdef HAL_I2C_MODULE_ENABLED
class HardI2cMaster : public I2cMaster, private PeripheralBase {
   public:
    explicit HardI2cMaster(I2C_HandleTypeDef& handle);
    ~HardI2cMaster();

   public:
    Result          setTransitionConfig(I2cMasterTransitionConfig& config) override;
    os::AsyncResult readReg(u16 regAddr, const Slice& data) override;
    os::AsyncResult writeReg(u16 regAddr, const Slice& data) override;
    os::AsyncResult read(const Slice& data) override;
    os::AsyncResult write(const Slice& data) override;

   private:
    static void onReadCplt(I2C_HandleTypeDef* instance);
    static void onWriteCplt(I2C_HandleTypeDef* instance);
    static void onError(I2C_HandleTypeDef* instance);

   private:
    I2C_HandleTypeDef*        _handle;
    I2cMasterTransitionConfig _transitionConfig;
    os::AsyncSource           _asyncSource;

   private:
#if CHIP_I2C_READ_DMA_ENABLED
#ifdef STM32H7xx
    Slice _rxUserBuffer;
#endif
#endif
};
#endif  // HAL_I2C_MODULE_ENABLED

}  // namespace wibot::hal

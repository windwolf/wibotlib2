#include "i2c.hpp"

#ifdef HAL_I2C_MODULE_ENABLED
namespace wibot {
HardI2cMaster::HardI2cMaster(I2C_HandleTypeDef& handle) : _handle(&handle) {
    HAL_I2C_RegisterCallback(_handle, HAL_I2C_MASTER_TX_COMPLETE_CB_ID,
                             &HardI2cMaster::onWriteCplt);
    HAL_I2C_RegisterCallback(_handle, HAL_I2C_MASTER_RX_COMPLETE_CB_ID,
                             &HardI2cMaster::onReadCplt);
    HAL_I2C_RegisterCallback(_handle, HAL_I2C_MEM_TX_COMPLETE_CB_ID, &HardI2cMaster::onWriteCplt);
    HAL_I2C_RegisterCallback(_handle, HAL_I2C_MEM_RX_COMPLETE_CB_ID, &HardI2cMaster::onReadCplt);
    HAL_I2C_RegisterCallback(_handle, HAL_I2C_ERROR_CB_ID, &HardI2cMaster::onError);
    PeripheralManager::getInstance().registerPeripheral(this, _handle);
};

HardI2cMaster::~HardI2cMaster() {
    HAL_I2C_UnRegisterCallback(_handle, HAL_I2C_MASTER_TX_COMPLETE_CB_ID);
    HAL_I2C_UnRegisterCallback(_handle, HAL_I2C_MASTER_RX_COMPLETE_CB_ID);
    HAL_I2C_UnRegisterCallback(_handle, HAL_I2C_MEM_TX_COMPLETE_CB_ID);
    HAL_I2C_UnRegisterCallback(_handle, HAL_I2C_MEM_RX_COMPLETE_CB_ID);
    HAL_I2C_UnRegisterCallback(_handle, HAL_I2C_ERROR_CB_ID);
    PeripheralManager::getInstance().unregisterPeripheral(this);
};

AsyncResult HardI2cMaster::readReg(u16 regAddr, const Slice& data) {
    if (HAL_I2C_GetState(_handle) != HAL_I2C_STATE_READY) {
        return AsyncResult::fromResult(Result::kBusy);
    }
#if CHIP_I2C_READ_DMA_ENABLED
#ifdef STM32H7xx
    _rxUserBuffer = data;
#endif
    auto rst = HAL_I2C_Mem_Read_DMA(_handle, _transitionConfig.deviceAddr << 1, regAddr,
                                    _transitionConfig.dataWidth == DataWidth::k8Bits
                                        ? I2C_MEMADD_SIZE_8BIT
                                        : I2C_MEMADD_SIZE_16BIT,
                                    data.data, data.size);
#endif
#if CHIP_I2C_READ_IT_ENABLED
    auto rst = HAL_I2C_Mem_Read_IT(_handle, _transitionConfig.deviceAddr << 1, regAddr,
                                   _transitionConfig.dataWidth == DataWidth::k8Bits
                                       ? I2C_MEMADD_SIZE_8BIT
                                       : I2C_MEMADD_SIZE_16BIT,
                                   data.data, data.size);
#endif
    if (rst != HAL_OK) {
        return AsyncResult::fromResult((Result)rst);
    }

    return _asyncSource.getResult();
};

AsyncResult HardI2cMaster::writeReg(u16 regAddr, const Slice& data) {
    if (HAL_I2C_GetState(_handle) != HAL_I2C_STATE_READY) {
        return AsyncResult::fromResult(Result::kBusy);
    }
#if CHIP_I2C_WRITE_DMA_ENABLED
#ifdef STM32H7xx
    SCB_CleanDCache_by_Addr((u32*)data.data, data.size);
#endif
    auto rst = HAL_I2C_Mem_Write_DMA(_handle, _transitionConfig.deviceAddr << 1, regAddr,
                                     _transitionConfig.dataWidth == DataWidth::k8Bits
                                         ? I2C_MEMADD_SIZE_8BIT
                                         : I2C_MEMADD_SIZE_16BIT,
                                     data.data, data.size);
#endif
#if CHIP_I2C_WRITE_IT_ENABLED
    auto rst = HAL_I2C_Mem_Write_IT(_handle, _transitionConfig.deviceAddr << 1, regAddr,
                                    _transitionConfig.dataWidth == DataWidth::k8Bits
                                        ? I2C_MEMADD_SIZE_8BIT
                                        : I2C_MEMADD_SIZE_16BIT,
                                    data.data, data.size);
#endif
    if (rst != HAL_OK) {
        return AsyncResult::fromResult((Result)rst);
    }

    return _asyncSource.getResult();
};

AsyncResult HardI2cMaster::read(const Slice& data) {
    if (HAL_I2C_GetState(_handle) != HAL_I2C_STATE_READY) {
        return AsyncResult::fromResult(Result::kBusy);
    }
#if CHIP_I2C_READ_DMA_ENABLED
#ifdef STM32H7xx
    _rxUserBuffer = data;
#endif
    auto rst = HAL_I2C_Master_Receive_DMA(_handle, _transitionConfig.deviceAddr << 1, data.data,
                                          data.size);
#endif
#if CHIP_I2C_READ_IT_ENABLED
    auto rst =
        HAL_I2C_Master_Receive_IT(_handle, _transitionConfig.deviceAddr << 1, data.data, data.size);
#endif

    if (rst != HAL_OK) {
        return AsyncResult::fromResult((Result)rst);
    }

    return _asyncSource.getResult();
};
AsyncResult HardI2cMaster::write(const Slice& data) {
    if (HAL_I2C_GetState(_handle) != HAL_I2C_STATE_READY) {
        return AsyncResult::fromResult(Result::kBusy);
    }
#if CHIP_I2C_WRITE_DMA_ENABLED
#ifdef STM32H7xx
    SCB_CleanDCache_by_Addr((u32*)data.data, data.size);
#endif
    auto rst =
        HAL_I2C_Master_Transmit_DMA(_handle, _transitionConfig.deviceAddr << 1, data.data, data.size);
#endif
#if CHIP_I2C_WRITE_IT_ENABLED
    auto rst = HAL_I2C_Master_Transmit_IT(_handle, _transitionConfig.deviceAddr << 1, data.data,
                                          data.size);
#endif
    if (rst != HAL_OK) {
        return AsyncResult::fromResult((Result)rst);
    }

    return _asyncSource.getResult();
}

void HardI2cMaster::onReadCplt(I2C_HandleTypeDef* instance) {
    auto perip = (HardI2cMaster*)PeripheralManager::getInstance().getPeripheral(instance);
    if (perip == nullptr) {
        return;
    }
#ifdef STM32H7xx
#if CHIP_I2C_READ_DMA_ENABLED
    SCB_InvalidateDCache_by_Addr(perip->_rxUserBuffer.data, perip->_rxUserBuffer.size);
#endif
#endif
    perip->_asyncSource.setDone();
};
void HardI2cMaster::onWriteCplt(I2C_HandleTypeDef* instance) {
    auto perip = (HardI2cMaster*)PeripheralManager::getInstance().getPeripheral(instance);
    if (perip == nullptr) {
        return;
    }
    perip->_asyncSource.setDone();
};
void HardI2cMaster::onError(I2C_HandleTypeDef* instance) {
    auto perip = (HardI2cMaster*)PeripheralManager::getInstance().getPeripheral(instance);
    if (perip == nullptr) {
        return;
    }
#ifdef STM32H7xx
#if CHIP_I2C_READ_DMA_ENABLED
    SCB_InvalidateDCache_by_Addr(perip->_rxUserBuffer.data, perip->_rxUserBuffer.size);
#endif
#endif
    perip->_asyncSource.setError(Result(HAL_I2C_GetError(instance)));
}

Result HardI2cMaster::setTransitionConfig(I2cMasterTransitionConfig& config) {
    ASSERT(config.dataWidth == DataWidth::k8Bits || config.dataWidth == DataWidth::k16Bits,
           "i2c data width must be 8bits or 16bits");
    _transitionConfig = config;
    //    HAL_I2C_DeInit(_handle);
    //    _handle->Init.AddressingMode =
    //        config.is10BitsAddr ? I2C_ADDRESSINGMODE_10BIT : I2C_ADDRESSINGMODE_7BIT;
    //    HAL_I2C_Init(_handle);
    return Result::kOk;
}

}  // namespace wibot

#endif

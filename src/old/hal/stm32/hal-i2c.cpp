#include "hal-i2c.hpp"

#ifdef HAL_I2C_MODULE_ENABLED
namespace wibot {

HardI2cMaster::HardI2cMaster(I2C_HandleTypeDef* handle) : _handle(handle) {
    Initializer::getInstance().registerInitialObject(this);
};

HardI2cMaster::~HardI2cMaster() {
    HAL_I2C_UnRegisterCallback(_handle, HAL_I2C_MEM_TX_COMPLETE_CB_ID);
    HAL_I2C_UnRegisterCallback(_handle, HAL_I2C_MEM_RX_COMPLETE_CB_ID);
    HAL_I2C_UnRegisterCallback(_handle, HAL_I2C_ERROR_CB_ID);
    PeripheralManager::getInstance().registerPeripheral(this, _handle);
};

void HardI2cMaster::_init() {
    HAL_I2C_RegisterCallback(_handle, HAL_I2C_MEM_TX_COMPLETE_CB_ID, &HardI2cMaster::onWriteCplt);
    HAL_I2C_RegisterCallback(_handle, HAL_I2C_MEM_RX_COMPLETE_CB_ID, &HardI2cMaster::onReadCplt);
    HAL_I2C_RegisterCallback(_handle, HAL_I2C_ERROR_CB_ID, &HardI2cMaster::onError);
    PeripheralManager::getInstance().registerPeripheral(this, _handle);
}
Result HardI2cMaster::readReg(u16 address, void* data, u32 size, WaitHandler& waitHandler) {
    if (this->_waitTrigger.isAttached()) {
        return Result::kBusy;
    }
    this->_waitTrigger.attach(waitHandler);
#if CHIP_I2C_READ_DMA_ENABLED
#ifdef STM32H7xx
    _rxBuffer.data = data;
    _rxBuffer.size = size;
#endif
    return (Result)HAL_I2C_Mem_Read_DMA(
        _handle, _transitionConfig.deviceAddr, address,
        _transitionConfig.dataWidth == DataWidth::k8Bits ? I2C_MEMADD_SIZE_8BIT
                                                         : I2C_MEMADD_SIZE_16BIT,
        (u8*)data, size * (toUnderlying(_transitionConfig.dataWidth)));
#endif
#if CHIP_I2C_READ_IT_ENABLED
    return (Result)HAL_I2C_Mem_Read_IT(
        _handle, _transitionConfig.deviceAddr << 1, address,
        _transitionConfig.dataWidth == DataWidth::k8Bits ? I2C_MEMADD_SIZE_8BIT
                                                         : I2C_MEMADD_SIZE_16BIT,
        (u8*)data, size * (toUnderlying(_transitionConfig.dataWidth)));
#endif
};
Result HardI2cMaster::writeReg(u16 address, void* data, u32 size, WaitHandler& waitHandler) {
    if (this->_waitTrigger.isAttached()) {
        return Result::kBusy;
    }
    this->_waitTrigger.attach(waitHandler);
#if CHIP_I2C_WRITE_DMA_ENABLED
#ifdef STM32H7xx
    _txBuffer.data = data;
    _txBuffer.size = size * (toUnderlying(_transitionConfig.dataWidth));
    SCB_CleanDCache_by_Addr((u32*)data, size * (toUnderlying(_transitionConfig.dataWidth)));
#endif
    return (Result)HAL_I2C_Mem_Write_DMA(
        &_handle, _transitionConfig.deviceAddr, address,
        _transitionConfig.dataWidth == DataWidth::k8Bits ? I2C_MEMADD_SIZE_8BIT
                                                         : I2C_MEMADD_SIZE_16BIT,
        (u8*)data, size * (toUnderlying(_transitionConfig.dataWidth)));
#endif
#if CHIP_I2C_WRITE_IT_ENABLED
    return (Result)HAL_I2C_Mem_Write_IT(
        _handle, _transitionConfig.deviceAddr << 1, address,
        _transitionConfig.dataWidth == DataWidth::k8Bits ? I2C_MEMADD_SIZE_8BIT
                                                         : I2C_MEMADD_SIZE_16BIT,
        (u8*)data, size * (toUnderlying(_transitionConfig.dataWidth)));
#endif
};
Result HardI2cMaster::read(void* data, u32 size, WaitHandler& waitHandler) {
    if (_transitionConfig.dataWidth > DataWidth::k16Bits) {
        return Result::kNotSupport;
    }
    if (this->_waitTrigger.isAttached()) {
        return Result::kBusy;
    }
    this->_waitTrigger.attach(waitHandler);
#if CHIP_I2C_READ_DMA_ENABLED
#ifdef STM32H7xx
    _rxBuffer.data = data;
    _rxBuffer.size = size * (toUnderlying(_transitionConfig.dataWidth));
#endif
    return (Result)HAL_I2C_Master_Receive_DMA(_handle, _transitionConfig.deviceAddr, (u8*)data,
                                              size * (toUnderlying(_transitionConfig.dataWidth)));
#endif
#if CHIP_I2C_READ_IT_ENABLED
    return (Result)HAL_I2C_Master_Receive_IT(_handle, _transitionConfig.deviceAddr << 1, (u8*)data,
                                             size * (toUnderlying(_transitionConfig.dataWidth)));
#endif
};
Result HardI2cMaster::write(void* data, u32 size, WaitHandler& waitHandler) {
    if (_transitionConfig.dataWidth > DataWidth::k16Bits) {
        return Result::kNotSupport;
    }
    if (this->_waitTrigger.isAttached()) {
        return Result::kBusy;
    }
    this->_waitTrigger.attach(waitHandler);
#if CHIP_I2C_WRITE_DMA_ENABLED
#ifdef STM32H7xx
    _txBuffer.data = data;
    _txBuffer.size = size * (toUnderlying(_config.dataWidth));
    SCB_CleanDCache_by_Addr((u32*)data, size * (toUnderlying(_config.dataWidth)));
#endif
    return (Result)HAL_I2C_Master_Transmit_DMA(&_handle, _deviceAddr, (u8*)data,
                                               size * (toUnderlying(_dataWidth)));
#endif
#if CHIP_I2C_WRITE_IT_ENABLED
    return (Result)HAL_I2C_Master_Transmit_IT(_handle, _transitionConfig.deviceAddr << 1, (u8*)data,
                                              size * (toUnderlying(_transitionConfig.dataWidth)));
#endif
}

void HardI2cMaster::onReadCplt(I2C_HandleTypeDef* instance) {
    auto perip = (HardI2cMaster*)PeripheralManager::getInstance().getPeripheral(instance);
#ifdef STM32H7xx
#if CHIP_I2C_READ_DMA_ENABLED
    SCB_InvalidateDCache_by_Addr(perip->_rxBuffer.data, perip->_rxBuffer.size);
#endif
#endif
    perip->_waitTrigger.setDone();
    perip->_waitTrigger.detach();
};
void HardI2cMaster::onWriteCplt(I2C_HandleTypeDef* instance) {
    auto perip = (HardI2cMaster*)PeripheralManager::getInstance().getPeripheral(instance);
    perip->_waitTrigger.setDone();
    perip->_waitTrigger.detach();
};
void HardI2cMaster::onError(I2C_HandleTypeDef* instance) {
    auto perip = (HardI2cMaster*)PeripheralManager::getInstance().getPeripheral(instance);
#ifdef STM32H7xx
#if CHIP_I2C_READ_DMA_ENABLED
    SCB_InvalidateDCache_by_Addr(perip->_rxBuffer.data, perip->_rxBuffer.size);
#endif
#endif
    perip->_waitTrigger.setError();
    perip->_waitTrigger.detach();
}

Result HardI2cMaster::setBaseConfig(I2cMasterBaseConfig& config) {
    //    HAL_I2C_DeInit(_handle);
    //    _handle->Init.NoStretchMode = config.stretch ? I2C_NOSTRETCH_DISABLE : I2C_NOSTRETCH_ENABLE;
    //    HAL_I2C_Init(_handle);
    return Result::kOk;
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

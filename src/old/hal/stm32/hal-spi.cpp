#include "hal-spi.hpp"

#include "peripheral.hpp"

#ifdef HAL_SPI_MODULE_ENABLED

namespace wibot {

struct SizeInfo {
   public:
    u32 sizeInBytes;
    u32 sizeInDMADataWidth;
    u32 sizeInSPIDataWidth;
};

void Spi::_onWriteCplt(SPI_HandleTypeDef* handle) {
    Spi* perip = (Spi*)PeripheralManager::getInstance().getPeripheral(handle);
#ifdef STM32H7xx
#if CHIP_SPI_READ_DMA_ENABLED
    SCB_InvalidateDCache_by_Addr(perip->_rxBuffer.data, perip->_rxBuffer.size);
#endif
#endif

    perip->_waitTrigger.setDone();
    perip->_waitTrigger.detach();
};

void Spi::_onReadCplt(SPI_HandleTypeDef* handle) {
    Spi* perip = (Spi*)PeripheralManager::getInstance().getPeripheral(handle);

#ifdef STM32H7xx
#if CHIP_SPI_READ_DMA_ENABLED
    SCB_InvalidateDCache_by_Addr(perip->_rxBuffer.data, perip->_rxBuffer.size);
#endif
#endif
    perip->_waitTrigger.setDone();
    perip->_waitTrigger.detach();
};

void Spi::_onWriteReadCplt(SPI_HandleTypeDef* handle) {
    Spi* perip = (Spi*)PeripheralManager::getInstance().getPeripheral(handle);

#ifdef STM32H7xx
#if CHIP_SPI_READ_DMA_ENABLED
    SCB_InvalidateDCache_by_Addr(perip->_rxBuffer.data, perip->_rxBuffer.size);
#endif
#endif
    perip->_waitTrigger.setDone();
    perip->_waitTrigger.detach();
};

void Spi::_onError(SPI_HandleTypeDef* handle) {
    Spi* perip = (Spi*)PeripheralManager::getInstance().getPeripheral(handle);

#ifdef STM32H7xx
#if CHIP_SPI_READ_DMA_ENABLED
    SCB_InvalidateDCache_by_Addr(perip->_rxBuffer.data, perip->_rxBuffer.size);
#endif
#endif
    perip->_waitTrigger.setError();
    perip->_waitTrigger.detach();
};

static void bits_switch(SPI_HandleTypeDef* handle, DataWidth dataWidth, u32 size,
                        SizeInfo& sizeInfo) {
    sizeInfo.sizeInBytes = size << (toUnderlying(dataWidth) - 1);
    switch (dataWidth) {
        case DataWidth::k8Bits:
        case DataWidth::k16Bits:
            sizeInfo.sizeInDMADataWidth = size;
            sizeInfo.sizeInSPIDataWidth = size;
            break;
        case DataWidth::k24Bits:
            sizeInfo.sizeInDMADataWidth = size * 3;
            sizeInfo.sizeInSPIDataWidth = size * 3;
            break;
        case DataWidth::k32Bits:
            sizeInfo.sizeInDMADataWidth = size << 1;
            sizeInfo.sizeInSPIDataWidth = size << 1;
            break;
        default:
            break;
    }

    u32 stream_number_tx = (((u32)((u32*)handle->hdmatx->Instance) & 0xFFU) - 0x08UL) / 0x014UL;
    u32 dma_base_tx = (u32)((u32*)handle->hdmatx->Instance) - stream_number_tx * 0x014UL - 0x08UL;
    u32 stream_number_rx = (((u32)((u32*)handle->hdmarx->Instance) & 0xFFU) - 0x08UL) / 0x014UL;
    u32 dma_base_rx = (u32)((u32*)handle->hdmarx->Instance) - stream_number_rx * 0x014UL - 0x08UL;

    switch (dataWidth) {
        case DataWidth::k8Bits:
        case DataWidth::k24Bits: {
            handle->Init.DataSize = SPI_DATASIZE_8BIT;
            LL_SPI_SetDataWidth(handle->Instance, LL_SPI_DATAWIDTH_8BIT);
            auto& init1               = handle->hdmatx->Init;
            init1.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
            init1.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
            init1.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
            init1.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
            LL_DMA_SetMemorySize((DMA_TypeDef*)dma_base_tx, stream_number_tx,
                                 LL_DMA_MDATAALIGN_BYTE);
            LL_DMA_SetMemorySize((DMA_TypeDef*)dma_base_rx, stream_number_rx,
                                 LL_DMA_MDATAALIGN_BYTE);
            LL_DMA_SetPeriphSize((DMA_TypeDef*)dma_base_tx, stream_number_tx,
                                 LL_DMA_PDATAALIGN_BYTE);
            LL_DMA_SetPeriphSize((DMA_TypeDef*)dma_base_rx, stream_number_rx,
                                 LL_DMA_PDATAALIGN_BYTE);
        }

        break;
        case DataWidth::k16Bits:
        case DataWidth::k32Bits: {
            handle->Init.DataSize = SPI_DATASIZE_16BIT;
            LL_SPI_SetDataWidth(handle->Instance, LL_SPI_DATAWIDTH_16BIT);
            auto& init2               = handle->hdmatx->Init;
            init2.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
            init2.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
            init2.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
            init2.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
            LL_DMA_SetMemorySize((DMA_TypeDef*)dma_base_tx, stream_number_tx,
                                 LL_DMA_MDATAALIGN_HALFWORD);
            LL_DMA_SetMemorySize((DMA_TypeDef*)dma_base_rx, stream_number_rx,
                                 LL_DMA_MDATAALIGN_HALFWORD);
            LL_DMA_SetPeriphSize((DMA_TypeDef*)dma_base_tx, stream_number_tx,
                                 LL_DMA_PDATAALIGN_HALFWORD);
            LL_DMA_SetPeriphSize((DMA_TypeDef*)dma_base_rx, stream_number_rx,
                                 LL_DMA_PDATAALIGN_HALFWORD);
        }

        break;
        default:
            break;
    }
};

Spi::Spi(SPI_HandleTypeDef* handle) : Spi(handle, nullptr) {};

Spi::Spi(SPI_HandleTypeDef* handle, Pin* cs) : _handle(handle), _cs(cs) {
    Initializer::getInstance().registerInitialObject(this);
};

void Spi::_init() {
    HAL_SPI_RegisterCallback(_handle, HAL_SPI_TX_COMPLETE_CB_ID, &_onWriteCplt);
    HAL_SPI_RegisterCallback(_handle, HAL_SPI_RX_COMPLETE_CB_ID, &_onReadCplt);
    HAL_SPI_RegisterCallback(_handle, HAL_SPI_TX_RX_COMPLETE_CB_ID, &_onWriteReadCplt);
    HAL_SPI_RegisterCallback(_handle, HAL_SPI_ERROR_CB_ID, &_onError);
    PeripheralManager::getInstance().registerPeripheral(this, _handle);
};

Spi::~Spi() {
    PeripheralManager::getInstance().unregisterPeripheral(this);
};

Result Spi::read(void* data, u32 size, WaitHandler& waitHandler) {
    if (_waitTrigger.isAttached()) {
        return Result::kBusy;
    }
    _waitTrigger.attach(waitHandler);

    SizeInfo sizeInfo;
    bits_switch(_handle, _config.dataWidth, size, sizeInfo);

    if (_cs != nullptr) {
        _cs->write(PinStatus::kSet);
    }
#if CHIP_SPI_READ_DMA_ENABLED
#ifdef STM32H7xx
    _rxBuffer.data = data;
    _rxBuffer.size = sizeInfo.sizeInBytes;
#endif
    return (Result)HAL_SPI_Receive_DMA(_handle, (u8*)data, sizeInfo.sizeInDMADataWidth);
#endif
#if CHIP_SPI_READ_IT_ENABLED
    return (Result)HAL_SPI_Receive_IT(_handle, (u8*)data, sizeInfo.sizeInSPIDataWidth);
#endif
};
Result Spi::write(void* data, u32 size, WaitHandler& waitHandler) {
    if (_waitTrigger.isAttached()) {
        return Result::kBusy;
    }
    _waitTrigger.attach(waitHandler);

    SizeInfo sizeInfo;
    bits_switch(_handle, _config.dataWidth, size, sizeInfo);

    if (_cs != nullptr) {
        _cs->write(PinStatus::kSet);
    }
#if CHIP_SPI_WRITE_DMA_ENABLED
#ifdef STM32H7xx
    _txBuffer.data = data;
    _txBuffer.size = sizeInfo.sizeInBytes;
    SCB_CleanDCache_by_Addr((u32*)data, sizeInfo.sizeInBytes);
#endif
    return (Result)HAL_SPI_Transmit_DMA(_handle, static_cast<u8*>(data),
                                        sizeInfo.sizeInDMADataWidth);
#endif
#if CHIP_SPI_WRITE_IT_ENABLED
    return (Result)HAL_SPI_Transmit_IT(_handle, static_cast<u8*>(data),
                                       sizeInfo.sizeInSPIDataWidth);
#endif
}

Result Spi::writeRead(void* txData, void* rxData, u32 size, WaitHandler& waitHandler) {
    if (_waitTrigger.isAttached()) {
        return Result::kBusy;
    }
    _waitTrigger.attach(waitHandler);
    SizeInfo sizeInfo;
    bits_switch(_handle, _config.dataWidth, size, sizeInfo);

    if (_cs != nullptr) {
        _cs->write(PinStatus::kSet);
    }
#if CHIP_SPI_WRITE_DMA_ENABLED
#ifdef STM32H7xx
    _txBuffer.data = data;
    _txBuffer.size = size * (ToUnderlying(_config.dataWidth));
    SCB_CleanDCache_by_Addr((u32*)data, size * (ToUnderlying(_config.dataWidth)));
#endif
    return (Result)HAL_SPI_TransmitReceive_DMA(
        _handle, static_cast<u8*>(txData), static_cast<u8*>(rxData), sizeInfo.sizeInDMADataWidth);
#endif
#if CHIP_SPI_WRITE_IT_ENABLED
    return (Result)HAL_SPI_TransmitReceive_IT(
        _handle, static_cast<u8*>(txData), static_cast<u8*>(rxData), sizeInfo.sizeInSPIDataWidth);
#endif
}
Result Spi::setConfig(SpiConfig& config) {
    ASSERT(config.dataWidth == DataWidth::k8Bits || config.dataWidth == DataWidth::k16Bits,
           "SPI only support 8 or 16 bits data width");
    HAL_SPI_DeInit(_handle);
    _handle->Init.Mode      = SPI_MODE_MASTER;
    _handle->Init.Direction = SPI_DIRECTION_2LINES;
    switch (config.dataWidth) {
        case DataWidth::k8Bits:
            _handle->Init.DataSize = SPI_DATASIZE_8BIT;
            break;
        case DataWidth::k16Bits:
            _handle->Init.DataSize = SPI_DATASIZE_16BIT;
            break;
        default:
            break;
    }
    switch (config.cpha) {
        case SpiCpha::k1Edge:
            _handle->Init.CLKPhase = SPI_PHASE_1EDGE;
            break;
        case SpiCpha::k2Edge:
            _handle->Init.CLKPhase = SPI_PHASE_2EDGE;
            break;
        default:
            break;
    }
    switch (config.cpol) {
        case SpiCpol::k0:
            _handle->Init.CLKPolarity = SPI_POLARITY_LOW;
            break;
        case SpiCpol::k1:
            _handle->Init.CLKPolarity = SPI_POLARITY_HIGH;
            break;
        default:
            break;
    }
    HAL_SPI_Init(_handle);
    return Result::kOk;
}

Result Spi::begin() {
    __HAL_SPI_ENABLE(_handle);
    if (_cs != nullptr) {
        _cs->write(PinStatus::kSet);
    }
    return Result::kOk;
}

Result Spi::end() {
    __HAL_SPI_DISABLE(_handle);
    if (_cs != nullptr) {
        _cs->write(PinStatus::kReset);
    }
    return Result::kOk;
}

};  // namespace wibot

#endif

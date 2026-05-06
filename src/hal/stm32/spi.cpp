#include "spi.hpp"
#include "type.hpp"

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
    if (perip == nullptr) {
        return;
    }

    perip->_asyncSource.setDone();
};

void Spi::_onReadCplt(SPI_HandleTypeDef* handle) {
    Spi* perip = (Spi*)PeripheralManager::getInstance().getPeripheral(handle);
    if (perip == nullptr) {
        return;
    }

#ifdef STM32H7xx
#if CHIP_SPI_READ_DMA_ENABLED
    SCB_InvalidateDCache_by_Addr(perip->_rxUserBuffer.data, perip->_rxUserBuffer.size);
#endif
#endif
    perip->_asyncSource.setDone();
};

void Spi::_onWriteReadCplt(SPI_HandleTypeDef* handle) {
    Spi* perip = (Spi*)PeripheralManager::getInstance().getPeripheral(handle);
    if (perip == nullptr) {
        return;
    }

#ifdef STM32H7xx
#if CHIP_SPI_READ_DMA_ENABLED
    SCB_InvalidateDCache_by_Addr(perip->_rxUserBuffer.data, perip->_rxUserBuffer.size);
#endif
#endif
    perip->_asyncSource.setDone();
};

void Spi::_onError(SPI_HandleTypeDef* handle) {
    Spi* perip = (Spi*)PeripheralManager::getInstance().getPeripheral(handle);
    if (perip == nullptr) {
        return;
    }

#ifdef STM32H7xx
#if CHIP_SPI_READ_DMA_ENABLED
    SCB_InvalidateDCache_by_Addr(perip->_rxUserBuffer.data, perip->_rxUserBuffer.size);
#endif
#endif
    perip->_asyncSource.setError(Result(HAL_SPI_GetError(handle)));
};

static void bits_switch(SPI_HandleTypeDef* handle, DataWidth dataWidth) {
#if defined(STM32G4) || defined(STM32H0)

    auto initDataSize = (dataWidth == DataWidth::k8Bits) ? SPI_DATASIZE_8BIT : SPI_DATASIZE_16BIT;
    auto llDataWidth =
        (dataWidth == DataWidth::k8Bits) ? LL_SPI_DATAWIDTH_8BIT : LL_SPI_DATAWIDTH_16BIT;
    auto initAlignment =
        (dataWidth == DataWidth::k8Bits) ? DMA_PDATAALIGN_BYTE : DMA_PDATAALIGN_HALFWORD;
    auto llAlign =
        (dataWidth == DataWidth::k8Bits) ? LL_DMA_MDATAALIGN_BYTE : LL_DMA_MDATAALIGN_HALFWORD;

    if (handle->hdmatx != nullptr) {
        auto txDmaBase         = handle->hdmatx->DmaBaseAddress;
        auto txDmaChannelIndex = (handle->hdmatx->ChannelIndex >> 2U) + 1;

        LL_DMA_SetMemorySize(txDmaBase, txDmaChannelIndex, llAlign);
        LL_DMA_SetPeriphSize(txDmaBase, txDmaChannelIndex, llAlign);

        auto& init1               = handle->hdmatx->Init;
        init1.MemDataAlignment    = initAlignment;
        init1.MemDataAlignment    = initAlignment;
        init1.PeriphDataAlignment = initAlignment;
        init1.PeriphDataAlignment = initAlignment;
    }

    if (handle->hdmarx != nullptr) {
        auto rxDmaBese         = handle->hdmarx->DmaBaseAddress;
        auto rxDmaChannelIndex = (handle->hdmarx->ChannelIndex >> 2U) + 1;

        LL_DMA_SetMemorySize(rxDmaBese, rxDmaChannelIndex, llAlign);
        LL_DMA_SetPeriphSize(rxDmaBese, rxDmaChannelIndex, llAlign);

        auto& init1               = handle->hdmarx->Init;
        init1.MemDataAlignment    = initAlignment;
        init1.MemDataAlignment    = initAlignment;
        init1.PeriphDataAlignment = initAlignment;
        init1.PeriphDataAlignment = initAlignment;
    }

    handle->Init.DataSize = initDataSize;
    LL_SPI_SetDataWidth(handle->Instance, llDataWidth);

#endif
};

Spi::Spi(SPI_HandleTypeDef& handle, Pin* csPin, const SpiConfig& config)
    : _handle(&handle), _csPin(csPin) {
    setConfig(config);
    HAL_SPI_RegisterCallback(&handle, HAL_SPI_TX_COMPLETE_CB_ID, &_onWriteCplt);
    HAL_SPI_RegisterCallback(&handle, HAL_SPI_RX_COMPLETE_CB_ID, &_onReadCplt);
    HAL_SPI_RegisterCallback(&handle, HAL_SPI_TX_RX_COMPLETE_CB_ID, &_onWriteReadCplt);
    HAL_SPI_RegisterCallback(&handle, HAL_SPI_ERROR_CB_ID, &_onError);
    PeripheralManager::getInstance().registerPeripheral(this, &handle);
};

Spi::~Spi() {
    HAL_SPI_UnRegisterCallback(_handle, HAL_SPI_TX_COMPLETE_CB_ID);
    HAL_SPI_UnRegisterCallback(_handle, HAL_SPI_RX_COMPLETE_CB_ID);
    HAL_SPI_UnRegisterCallback(_handle, HAL_SPI_TX_RX_COMPLETE_CB_ID);
    HAL_SPI_UnRegisterCallback(_handle, HAL_SPI_ERROR_CB_ID);
    PeripheralManager::getInstance().unregisterPeripheral(this);
};

AsyncResult Spi::read(const Slice& data) {
#if CHIP_SPI_READ_DMA_ENABLED
#ifdef STM32H7xx
    _rxUserBuffer = data;
#endif
    auto rst = HAL_SPI_Receive_DMA(_handle, data.data,
                                   data.size >> (static_cast<u8>(_config.dataWidth) - 1));
#endif
#if CHIP_SPI_READ_IT_ENABLED
    auto rst = HAL_SPI_Receive_IT(_handle, data.data,
                                  data.size >> (static_cast<u8>(_config.dataWidth) - 1));
#endif

    if (rst != HAL_OK) {
        return AsyncResult::fromResult((Result)rst);
    }

    return _asyncSource.getResult();
};

AsyncResult Spi::write(const Slice& data) {
#if CHIP_SPI_WRITE_DMA_ENABLED
#ifdef STM32H7xx
    SCB_CleanDCache_by_Addr((u32*)data.data, data.size);
#endif
    auto rst = HAL_SPI_Transmit_DMA(_handle, data.data,
                                    data.size >> (static_cast<u8>(_config.dataWidth) - 1));
#endif
#if CHIP_SPI_WRITE_IT_ENABLED
    auto rst = HAL_SPI_Transmit_IT(_handle, data.data,
                                   data.size >> (static_cast<u8>(_config.dataWidth) - 1));
#endif

    if (rst != HAL_OK) {
        return AsyncResult::fromResult((Result)rst);
    }

    return _asyncSource.getResult();
}

AsyncResult Spi::writeRead(const Slice& txData, const Slice& rxData) {
#if CHIP_SPI_WRITE_DMA_ENABLED
#ifdef STM32H7xx
    _rxUserBuffer = rxData;
    SCB_CleanDCache_by_Addr((u32*)txData.data, txData.size);
#endif
    auto rst = HAL_SPI_TransmitReceive_DMA(_handle, txData.data, rxData.data,
                                           rxData.size >> (static_cast<u8>(_config.dataWidth) - 1));
#endif
#if CHIP_SPI_WRITE_IT_ENABLED
    auto rst = HAL_SPI_TransmitReceive_IT(_handle, txData.data, rxData.data,
                                          rxData.size >> (static_cast<u8>(_config.dataWidth) - 1));
#endif

    if (rst != HAL_OK) {
        return AsyncResult::fromResult((Result)rst);
    }

    return _asyncSource.getResult();
}
Result Spi::setConfig(const SpiConfig& config) {
    ASSERT(config.dataWidth == DataWidth::k8Bits || config.dataWidth == DataWidth::k16Bits,
           "SPI only support 8 or 16 bits data width");
    _config = config;
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

    bits_switch(_handle, _config.dataWidth);
    HAL_SPI_Init(_handle);
    return Result::kOk;
}

Result Spi::begin() {
    if (_csPin != nullptr) _csPin->setValue(false);
    return Result::kOk;
}

Result Spi::end() {
    if (_csPin != nullptr) _csPin->setValue(true);
    return Result::kOk;
}

};  // namespace wibot

#endif

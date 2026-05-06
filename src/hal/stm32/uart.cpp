#include "uart.hpp"

#include "logger.hpp"

#ifdef HAL_UART_MODULE_ENABLED

namespace wibot {
LOGGER("uart")

void Uart::_onWriteCplt(UART_HandleTypeDef *handle) {
    auto perip = (Uart *)PeripheralManager::getInstance().getPeripheral(handle);
    if (perip == nullptr) {
        return;
    }
    perip->txCount++;
    perip->_txAsyncSource.setDone();
};

void Uart::_onReadCplt(UART_HandleTypeDef *handle) {
    auto perip = (Uart *)PeripheralManager::getInstance().getPeripheral(handle);
    if (perip == nullptr) {
        return;
    }
    auto rxUserBuffer = perip->_rxUserBuffer;
    if (rxUserBuffer != nullptr) {
#ifdef STM32H7xx
#if CHIP_UART_READ_DMA_ENABLED
        SCB_InvalidateDCache_by_Addr(rxUserBuffer->data, rxUserBuffer->size);
#endif
#endif
        perip->rxCount++;
        perip->_rxUserBuffer = nullptr;
        perip->_rxAsyncSource.setDone();
    }
};

void Uart::_onCircularDataReceived(UART_HandleTypeDef *handle, u16 pos) {
    auto perip = (Uart *)PeripheralManager::getInstance().getPeripheral(handle);
    if (perip == nullptr || perip->_rxBuffer == nullptr) {
        return;
    }
    auto rxBuffer = perip->_rxBuffer;
#ifdef STM32H7xx
#if CHIP_UART_READ_DMA_ENABLED
    SCB_InvalidateDCache_by_Addr((u8 *)rxBuffer->getDataPtr(), rxBuffer->getMemCapacity());
#endif
#endif
    perip->rxCount++;

#if CHIP_UART_READ_DMA_ENABLED
    auto isFullTransferFromStart =
        pos == rxBuffer->getMemCapacity() && perip->_lastPos == 0;
    auto length = isFullTransferFromStart ? rxBuffer->getMemCapacity()
                                          : rxBuffer->getLengthByMemIndex(pos, perip->_lastPos);
#else
    auto length = rxBuffer->getLengthByMemIndex(pos, perip->_lastPos);
#endif
    perip->_lastPos = pos;
    rxBuffer->writeVirtual(length);
    perip->_rxAsyncSource.setDone();
};

void Uart::_onError(UART_HandleTypeDef *handle) {
    Uart *perip = (Uart *)PeripheralManager::getInstance().getPeripheral(handle);
    if (perip == nullptr) {
        return;
    }
#ifdef STM32H7xx
#if CHIP_UART_READ_DMA_ENABLED
    if (perip->_rxBuffer != nullptr) {
        SCB_InvalidateDCache_by_Addr((u8 *)perip->_rxBuffer->getDataPtr(),
                                     perip->_rxBuffer->getMemCapacity());
    }
    if (perip->_rxUserBuffer != nullptr) {
        SCB_InvalidateDCache_by_Addr(perip->_rxUserBuffer->data, perip->_rxUserBuffer->size);
    }
#endif
#endif
    if (!perip->_config.notIgnoreOverrunError) {
        LL_USART_ClearFlag_ORE(handle->Instance);
    }
    if (!perip->_config.notIgnoreFrameError) {
        LL_USART_ClearFlag_FE(handle->Instance);
    }
    if (!perip->_config.notIgnoreNoiseError) {
        LL_USART_ClearFlag_NE(handle->Instance);
    }
    if (!perip->_config.notIgnoreParityError) {
        LL_USART_ClearFlag_PE(handle->Instance);
    }
    // TODO: clear related error flag, according to error ignore config.

    auto errCode = HAL_UART_GetError(handle);
    auto result  = Result(Result::ResultStatus::kError, errCode);
#ifdef STM32F1xx
    LOG_E("%s: on error ISR=%lX,err=%lX,uec=%lu", perip->_name, perip->_handle->Instance->SR,
          HAL_UART_GetError(handle), perip->errorCount);
#else
    LOG_E("%s: on error ISR=%X,err=%X,uec=%u", perip->_name,
          (unsigned int)perip->_handle->Instance->ISR, (unsigned int)HAL_UART_GetError(handle),
          (unsigned int)perip->errorCount);
#endif
    perip->errorCount++;
    perip->_lastPos      = 0;
    perip->_readedLength = 0;
    perip->_rxUserBuffer = nullptr;

    perip->_rxAsyncSource.setError(result);

    perip->_txAsyncSource.setError(result);
};

Uart::Uart(UART_HandleTypeDef &handle, const char *name, CircularBuffer8 *rxBuffer)
    : _handle(&handle),
      _name(name),

      _txAsyncSource(),
      _rxAsyncSource(),
      _rxBuffer(rxBuffer),
      _rxUserBuffer(nullptr),
      _lastPos(0),
      _readedLength(0),

      errorCount(0),
      rxCount(0),
      txCount(0) {
    if (_isCircularMode()) {
        HAL_UART_RegisterRxEventCallback(&handle, &_onCircularDataReceived);
    } else {
        HAL_UART_RegisterCallback(&handle, HAL_UART_RX_COMPLETE_CB_ID, &_onReadCplt);
    }
    HAL_UART_RegisterCallback(&handle, HAL_UART_TX_COMPLETE_CB_ID, &_onWriteCplt);
    HAL_UART_RegisterCallback(&handle, HAL_UART_ERROR_CB_ID, &_onError);
    PeripheralManager::getInstance().registerPeripheral(this, &handle);
};

Uart::~Uart() {
    PeripheralManager::getInstance().unregisterPeripheral(this);
    if (_isCircularMode()) {
        HAL_UART_UnRegisterRxEventCallback(_handle);
    } else {
        HAL_UART_UnRegisterCallback(_handle, HAL_UART_RX_COMPLETE_CB_ID);
    }
    HAL_UART_UnRegisterCallback(_handle, HAL_UART_TX_COMPLETE_CB_ID);
    HAL_UART_UnRegisterCallback(_handle, HAL_UART_ERROR_CB_ID);
};

bool Uart::_isCircularMode() const {
    return _rxBuffer != nullptr;
};

bool Uart::_isRxTransferArmed() const {
    if (_handle->ReceptionType != HAL_UART_RECEPTION_TOIDLE) {
        return false;
    }

#if CHIP_UART_READ_DMA_ENABLED
    if (_handle->hdmarx == nullptr) {
        return false;
    }
    return HAL_IS_BIT_SET(_handle->Instance->CR3, USART_CR3_DMAR) &&
           HAL_IS_BIT_SET(_handle->hdmarx->Instance->CCR, DMA_CCR_EN) &&
           HAL_IS_BIT_SET(_handle->Instance->CR1, USART_CR1_IDLEIE);
#else
    return _handle->RxISR != nullptr && HAL_IS_BIT_SET(_handle->Instance->CR1, USART_CR1_IDLEIE);
#endif
}

void Uart::_clearRxErrorFlags() {
    LL_USART_ClearFlag_ORE(_handle->Instance);
    LL_USART_ClearFlag_FE(_handle->Instance);
    LL_USART_ClearFlag_NE(_handle->Instance);
    LL_USART_ClearFlag_PE(_handle->Instance);
    LL_USART_ClearFlag_IDLE(_handle->Instance);
#ifdef USART_ICR_CMCF
    LL_USART_ClearFlag_CM(_handle->Instance);
#endif
#ifdef USART_ICR_RTOCF
    LL_USART_ClearFlag_RTO(_handle->Instance);
#endif
#ifdef UART_RXDATA_FLUSH_REQUEST
    __HAL_UART_SEND_REQ(_handle, UART_RXDATA_FLUSH_REQUEST);
#endif
    _handle->ErrorCode = HAL_UART_ERROR_NONE;
}

AsyncResult Uart::subscribe() {
    return _rxAsyncSource.getResult(true);
};

AsyncResult Uart::read(const Slice &data) {
    if (_isCircularMode()) {
        return AsyncResult::fromError(Result::kNotSupport);
    }

    if ((HAL_UART_GetState(_handle) & HAL_UART_STATE_BUSY_RX) == HAL_UART_STATE_BUSY_RX ||
        _rxUserBuffer != nullptr) {
        return AsyncResult::fromError(Result::kBusy);
    }

    _clearRxErrorFlags();
    _rxAsyncSource.reset();
    _rxUserBuffer = &data;

    HAL_StatusTypeDef rst = HAL_ERROR;
#if CHIP_UART_READ_DMA_ENABLED
    rst = HAL_UART_Receive_DMA(_handle, (u8 *)_rxUserBuffer->data, (u16)_rxUserBuffer->size);
#endif
#if CHIP_UART_READ_IT_ENABLED
    rst = HAL_UART_Receive_IT(_handle, (u8 *)data.data, (u16)data.size);
#endif
    if (rst != HAL_OK) {
        _rxUserBuffer = nullptr;
        return AsyncResult::fromResult((Result)rst);
    }
    return _rxAsyncSource.getResult();
};

AsyncResult Uart::write(const Slice &data) {
    if ((HAL_UART_GetState(_handle) & HAL_UART_STATE_BUSY_TX) == HAL_UART_STATE_BUSY_TX) {
        return AsyncResult::fromResult(Result::kBusy);
    }

    //_txUserBuffer = &data;
    _txAsyncSource.reset();

#if CHIP_UART_WRITE_DMA_ENABLED
#ifdef STM32H7xx
    SCB_CleanDCache_by_Addr((u32 *)data.data, data.size);
#endif
    auto rst = HAL_UART_Transmit_DMA(_handle, (u8 *)data.data, (u16)data.size);
#endif
#if CHIP_UART_WRITE_IT_ENABLED
    auto rst = HAL_UART_Transmit_IT(_handle, (u8 *)data.data, (u16)data.size);
#endif

    if (rst != HAL_OK) {
        return AsyncResult::fromResult((Result)rst);
    }
    return _txAsyncSource.getResult();
};

Result Uart::start() {
    if (!_isCircularMode()) {
        return Result::kNotSupport;
    }
#if CHIP_UART_READ_DMA_ENABLED
    if (_handle->hdmarx == nullptr || _handle->hdmarx->Init.Mode != DMA_CIRCULAR) {
        return Result::kNotSupport;
    }
#endif
    if ((HAL_UART_GetState(_handle) & HAL_UART_STATE_BUSY_RX) == HAL_UART_STATE_BUSY_RX) {
        if (_isRxTransferArmed()) {
            return Result::kBusy;
        }
        LOG_W("%s: recover stale rx state.", _name);
        Result abortResult = (Result)HAL_UART_AbortReceive(_handle);
        if (!abortResult.isOk()) {
            return abortResult;
        }
    }

    _clearRxErrorFlags();
    _rxAsyncSource.reset();
    _lastPos      = 0;
    _readedLength = 0;

#if CHIP_UART_READ_DMA_ENABLED
    auto rst = HAL_UARTEx_ReceiveToIdle_DMA(_handle, (u8 *)_rxBuffer->getDataPtr(),
                                            _rxBuffer->getMemCapacity());
#else
    auto rst = HAL_UARTEx_ReceiveToIdle_IT(_handle, (u8 *)_rxBuffer->getDataPtr(),
                                           _rxBuffer->getMemCapacity());
#endif
    if (rst != HAL_OK) {
        HAL_UART_AbortReceive(_handle);
        _clearRxErrorFlags();
    }
    return (Result)rst;
};

Result Uart::stop() {
    Result rst = (Result)HAL_UART_AbortReceive(_handle);
    _clearRxErrorFlags();
    _lastPos      = 0;
    _readedLength = 0;
    _rxUserBuffer = nullptr;
    _rxAsyncSource.setDone();
    return rst;
}

Result Uart::setConfig(UartConfig &config) {
    _config = config;
    HAL_UART_DeInit(_handle);
    _handle->Init.BaudRate = config.baudrate;
    switch (config.wordLength) {
#ifndef STM32F1xx
        case UartWordLength::k7B:
            _handle->Init.WordLength = UART_WORDLENGTH_7B;
            break;
#endif
        case UartWordLength::k8B:
            _handle->Init.WordLength = UART_WORDLENGTH_8B;
            break;
        case UartWordLength::k9B:
            _handle->Init.WordLength = UART_WORDLENGTH_9B;
            break;
        default:
            return Result::kNotSupport;
    }
    switch (config.stopBits) {
#ifndef STM32F1xx
        case UartStopBits::k0_5:
            _handle->Init.StopBits = UART_STOPBITS_0_5;
            break;
#endif
        case UartStopBits::k1:
            _handle->Init.StopBits = UART_STOPBITS_1;
            break;
#ifndef STM32F1xx
        case UartStopBits::k1_5:
            _handle->Init.StopBits = UART_STOPBITS_1_5;
            break;
#endif
        case UartStopBits::k2:
            _handle->Init.StopBits = UART_STOPBITS_2;
            break;
        default:
            return Result::kNotSupport;
    }
    switch (config.parity) {
        case UartParity::kNone:
            _handle->Init.Parity = UART_PARITY_NONE;
            break;
        case UartParity::KEven:
            _handle->Init.Parity = UART_PARITY_EVEN;
            break;
        case UartParity::KOdd:
            _handle->Init.Parity = UART_PARITY_ODD;
            break;
        default:
            return Result::kNotSupport;
    }
    HAL_UART_Init(_handle);
    return Result::kOk;
}

};  // namespace wibot

void UartSendByte(const char *data, u16 len) {
    for (u16 todo = 0; todo < len; todo++) {
        /* 堵塞判断串口是否发送完成 */
        while (LL_USART_IsActiveFlag_TC(USART1) == 0)
            ;

        /* 串口发送完成，将该字符发送 */
        LL_USART_TransmitData8(USART1, (u8)*data++);
    }
}

#endif  // HAL_UART_MODULE_ENABLED

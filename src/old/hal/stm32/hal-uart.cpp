#include "hal-uart.hpp"

#include "peripheral.hpp"
#include "logger.hpp"

#ifdef HAL_UART_MODULE_ENABLED

namespace wibot {
LOGGER("uart")

void Uart::_onWriteCplt(UART_HandleTypeDef *handle) {
    auto perip = (Uart *)PeripheralManager::getInstance().getPeripheral(handle);
    perip->txCount++;
#ifdef STM32H7xx
#if CHIP_UART_READ_DMA_ENABLED
    perip->_txBuffer.data = nullptr;
    perip->_txBuffer.size = 0;
#endif
#endif
    perip->_writeWaitTrigger.setDone();
    perip->_writeWaitTrigger.detach();
};

void Uart::_onReadCplt(UART_HandleTypeDef *handle) {
    auto perip = (Uart *)PeripheralManager::getInstance().getPeripheral(handle);
#ifdef STM32H7xx
#if CHIP_UART_READ_DMA_ENABLED
    SCB_InvalidateDCache_by_Addr(perip->_rxBuffer.data, perip->_rxBuffer.size);
    perip->_rxBuffer.data = nullptr;
    perip->_rxBuffer.size = 0;
#endif
#endif
    perip->rxCount++;
    perip->_readWaitTrigger.setDone();
    perip->_readWaitTrigger.detach();
};

void Uart::_onCircularDataReceived(UART_HandleTypeDef *handle, u16 pos) {
    auto perip = (Uart *)PeripheralManager::getInstance().getPeripheral(handle);
#ifdef STM32H7xx
#if CHIP_UART_READ_DMA_ENABLED
    SCB_InvalidateDCache_by_Addr((u8 *)perip->cirRxBuffer_.getDataPtr(),
                                 perip->cirRxBuffer_.getMemCapacity());
    if (!perip->_isListenMode) {
        perip->_rxBuffer.data = nullptr;
        perip->_rxBuffer.size = 0;
    }
#endif
#endif
    if (perip->_isListenMode) {
        auto length = perip->_rxCb->getLengthByMemIndex(pos, perip->_lastPos);
        perip->rxCount++;
        perip->_lastPos = pos;
        perip->_rxCb->writeVirtual(length);
        perip->_readWaitTrigger.setDone();
    } else {
        perip->rxCount++;
        perip->_readLength = pos;
        perip->_readWaitTrigger.setDone();
        perip->_readWaitTrigger.detach();
    }
};

void Uart::_onError(UART_HandleTypeDef *handle) {
    Uart *perip = (Uart *)PeripheralManager::getInstance().getPeripheral(handle);
#ifdef STM32H7xx
#if CHIP_UART_READ_DMA_ENABLED
    if (perip->cirRxBuffer_ != nullptr) {
        SCB_InvalidateDCache_by_Addr((u8 *)perip->cirRxBuffer_.getDataPtr(),
                                     perip->cirRxBuffer_.getMemCapacity());
    }
    if (perip->_txBuffer.data != nullptr) {
        SCB_InvalidateDCache_by_Addr(perip->_txBuffer.data, perip->_txBuffer.size);
        perip->_txBuffer.data = nullptr;
        perip->_txBuffer.size = 0;
    }
    if (perip->_rxBuffer.data != nullptr) {
        SCB_InvalidateDCache_by_Addr(perip->_rxBuffer.data, perip->_rxBuffer.size);
        perip->_rxBuffer.data = nullptr;
        perip->_rxBuffer.size = 0;
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

#ifdef STM32F1xx
    LOG_E("%s: on error ISR=%lX,err=%X,uec=%lu", perip->_name, perip->_handle->Instance->SR,
          HAL_UART_GetError(handle), perip->errorCount);
#else
    LOG_E("%s: on error ISR=%X,err=%X,uec=%d", perip->_name, perip->_handle->Instance->ISR,
          HAL_UART_GetError(handle), perip->errorCount);
#endif
    perip->errorCount++;
    perip->_lastPos = 0;

    perip->_readWaitTrigger.setError();
    perip->_readWaitTrigger.detach();

    perip->_writeWaitTrigger.setError();
    perip->_writeWaitTrigger.detach();
};

Uart::Uart(UART_HandleTypeDef *handle, const char *name, bool listenMode)
    : _handle(handle), _name(name), _isListenMode(listenMode) {
    Initializer::getInstance().registerInitialObject(this);
};

void Uart::_init() {
    HAL_UART_RegisterCallback(_handle, HAL_UART_TX_COMPLETE_CB_ID, &_onWriteCplt);
    HAL_UART_RegisterCallback(_handle, HAL_UART_RX_COMPLETE_CB_ID, &_onReadCplt);

    HAL_UART_RegisterRxEventCallback(_handle, &_onCircularDataReceived);

    HAL_UART_RegisterCallback(_handle, HAL_UART_ERROR_CB_ID, &_onError);
    PeripheralManager::getInstance().registerPeripheral(this, _handle);
};

Uart::~Uart() {
    PeripheralManager::getInstance().unregisterPeripheral(this);
};

Result Uart::read(void *data, u32 size, WaitHandler &waitHandler) {
    if (_isListenMode) {
        // If the UART is in listening mode, we cannot read directly.
        return Result::kNotSupport;
    }

    if (_readWaitTrigger.isAttached()) {
        return Result::kBusy;
    }

    if ((HAL_UART_GetState(_handle) & HAL_UART_STATE_BUSY_RX) == HAL_UART_STATE_BUSY_RX) {
        return Result::kBusy;
    }
    _readWaitTrigger.attach(waitHandler);

#if CHIP_UART_READ_DMA_ENABLED
#ifdef STM32H7xx
    _rxBuffer.data = data;
    _rxBuffer.size = size;
#endif
    return (Result)HAL_UART_Receive_DMA(_handle, (u8 *)data, (u16)size);
#endif
#if CHIP_UART_READ_IT_ENABLED
    return (Result)HAL_UART_Receive_IT(_handle, (u8 *)data, (u16)size);
#endif
};
Result Uart::write(void *data, u32 size, WaitHandler &waitHandler) {
    if (_writeWaitTrigger.isAttached()) {
        return Result::kBusy;
    }

    if ((HAL_UART_GetState(_handle) & HAL_UART_STATE_BUSY_TX) == HAL_UART_STATE_BUSY_TX) {
        return Result::kBusy;
    }
    _writeWaitTrigger.attach(waitHandler);

#if CHIP_UART_WRITE_DMA_ENABLED
#ifdef STM32H7xx
    SCB_CleanDCache_by_Addr((u32 *)data, size);
#endif
    return (Result)HAL_UART_Transmit_DMA(_handle, (u8 *)data, (u16)size);
#endif
#if CHIP_UART_WRITE_IT_ENABLED
    return (Result)HAL_UART_Transmit_IT(_handle, (u8 *)data, (u16)size);
#endif
};

Result Uart::listen(CircularBuffer<u8> &rxBuffer, WaitHandler &waitHandler) {
    if (_isListenMode == false) {
        return Result::kNotSupport;
    }

    if (_readWaitTrigger.isAttached()) {
        return Result::kBusy;
    }
    if (_handle->hdmarx->Init.Mode != DMA_CIRCULAR) {
        return Result::kNotSupport;
    }
    if ((HAL_UART_GetState(_handle) & HAL_UART_STATE_BUSY_RX) == HAL_UART_STATE_BUSY_RX) {
        return Result::kBusy;
    }

    _readWaitTrigger.attach(waitHandler);
    _rxCb = &rxBuffer;
#if CHIP_UART_READ_DMA_ENABLED
    return (Result)HAL_UARTEx_ReceiveToIdle_DMA(_handle, (u8 *)rxBuffer.getDataPtr(),
                                                rxBuffer.getMemCapacity());
#else
    return (Result)HAL_UARTEx_ReceiveToIdle_IT(_handle, (u8 *)rxBuffer.getDataPtr(),
                                               rxBuffer.getMemCapacity());
#endif
};

Result Uart::stop() {
    Result rst = Result::kOk;

#if CHIP_UART_READ_DMA_ENABLED
    rst = (Result)HAL_UART_DMAStop(_handle);
#else
    rst = (Result)HAL_UART_AbortReceive_IT(_handle);
    ;
#endif
    HAL_UART_Abort(_handle);
    _lastPos = 0;

    _readWaitTrigger.setDone();
    _readWaitTrigger.detach();
    if (_rxCb != nullptr) {
        _rxCb = nullptr;
    }
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

Result Uart::readToIdle(void *data, u32 size, WaitHandler &waitHandler) {
    if (_isListenMode) {
        // If the UART is in listening mode, we cannot read directly.
        return Result::kNotSupport;
    }

    if (_readWaitTrigger.isAttached()) {
        return Result::kBusy;
    }

    if ((HAL_UART_GetState(_handle) & HAL_UART_STATE_BUSY_RX) == HAL_UART_STATE_BUSY_RX) {
        return Result::kBusy;
    }
    _readWaitTrigger.attach(waitHandler);

#if CHIP_UART_READ_DMA_ENABLED
#ifdef STM32H7xx
    _rxBuffer.data = (u8 *)data;
    _rxBuffer.size = size;
#endif
    return (Result)HAL_UARTEx_ReceiveToIdle_DMA(_handle, (u8 *)data, (u16)size);
#endif
#if CHIP_UART_READ_IT_ENABLED
    return (Result)HAL_UARTEx_ReceiveToIdle_IT(_handle, (u8 *)data, (u16)size);
#endif
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

#pragma once

//
// Created by zhouj on 2023/9/13.
//

#include "circular-buffer.hpp"
#include "peripheral.hpp"
#include "wait-handler.hpp"
#include "bus.hpp"
#include "buffer.hpp"

namespace wibot {

class Uart : public UartBus, private PeripheralBase, private Initializable {
   public:
    Uart(UART_HandleTypeDef* handle, const char* name, bool listenMode = true);
    ~Uart();
    Result setConfig(UartConfig& config) override;
    Result read(void* data, u32 size, WaitHandler& waitHandler);
    Result readToIdle(void* data, u32 size, WaitHandler& waitHandler);
    u16    getReadLength() const {
        return _readLength;
    }
    Result write(void* data, u32 size, WaitHandler& waitHandler) override;
    /**
     * To use listen, underlying UART must be configured Rx with DMA.
     * @param rxBuffer
     * @param waitHandler
     * @return
     */
    Result listen(CircularBuffer<u8>& rxBuffer, WaitHandler& waitHandler) override;
    Result stop() override;

    const char* getName() const {
        return _name;
    }

   private:
    void _init() override;

   public:
    u32 errorCount;
    u32 rxCount;
    u32 txCount;

   private:
    UART_HandleTypeDef* _handle;
    const char*         _name;
    bool                _isListenMode;
    UartConfig          _config;
    WaitTrigger         _writeWaitTrigger;
    WaitTrigger         _readWaitTrigger;
    CircularBuffer8*    _rxCb;
    u16                 _lastPos;
    u16                 _readLength;
#ifdef STM32H7xx
    Slice _rxBuffer;
    Slice _txBuffer;
#endif

   protected:
    static void _onReadCplt(UART_HandleTypeDef* handle);
    static void _onWriteCplt(UART_HandleTypeDef* handle);
    static void _onCircularDataReceived(UART_HandleTypeDef* handle, u16 pos);
    static void _onError(UART_HandleTypeDef* handle);
};

}  // namespace wibot

#ifdef __cplusplus
extern "C" {
#endif
void UartSendByte(const char* data, u16 len);
#ifdef __cplusplus
}
#endif

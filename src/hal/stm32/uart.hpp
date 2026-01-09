#pragma once

#include "chip.hpp"
#include "buffer.hpp"
#include "circular-buffer.hpp"
#include "peripheral.hpp"
#include "bus.hpp"

#ifdef HAL_UART_MODULE_ENABLED
namespace wibot {

/**
 * @brief UART的流式处理
 * 
 */
class Uart : public UartStream, public PeripheralBase {
   public:
    Uart(UART_HandleTypeDef &handle, const char *name, CircularBuffer8 &rxBuffer);
    Uart(UART_HandleTypeDef &handle, const char *name);
    ~Uart();
    Result setConfig(UartConfig &config) override;

    AsyncResult subscribe() override;
    /**
   * To use listen, underlying UART must be configured Rx with DMA.
   * @return
   */
    Result      start() override;
    Result      stop() override;

    /**
     * @brief 读取rx数据. 直到填满data缓冲区为止才发出完成通知.
     * @note `read`和`readNotify`都不能并行使用
     * @note 调用者需确保在异步通知到达前, data缓冲区不会被修改或释放.
     * @param data 
     * @return AsyncResult 
     */
    AsyncResult read(const Slice &data) override;
    /**
     * @brief 写入tx数据, 直到完成才发出完成通知.
     * @note `write`不能并行使用
     * @note 调用者需确保在异步通知到达前, data缓冲区不会被修改或释放.
     * @param data 
     * @return AsyncResult 
     */
    AsyncResult write(const Slice &data) override;
    // /**
    //  * @brief 发起内部缓存中tx数据的实际发送. 当发送完成后, 产生通知.
    //  * @note `write`和`writeNotify`都不能并行使用
    //  * @return AsyncResult
    //  */
    // AsyncResult writeNotify() override;

    const char *getName() const {
        return _name;
    }

   private:
    bool _isCircularMode() const;

   private:
    UART_HandleTypeDef *_handle;
    const char         *_name;
    UartConfig          _config;
    AsyncSource         _txAsyncSource;
    AsyncSource         _rxAsyncSource;
    //CircularBuffer8    &_txBuffer;
    CircularBuffer8    *_rxBuffer;
    // const Slice       *_txUserBuffer;
    const Slice        *_rxUserBuffer;
    u16                 _lastPos;
    u16                 _readedLength;

   public:
    u32 errorCount;
    u32 rxCount;
    u32 txCount;

   protected:
    static void _onReadCplt(UART_HandleTypeDef *handle);
    static void _onWriteCplt(UART_HandleTypeDef *handle);
    static void _onCircularDataReceived(UART_HandleTypeDef *handle, u16 pos);
    static void _onError(UART_HandleTypeDef *handle);
};

}  // namespace wibot

#ifdef __cplusplus
extern "C" {
#endif
void UartSendByte(const char *data, u16 len);
#ifdef __cplusplus
}
#endif

#endif  // HAL_UART_MODULE_ENABLED

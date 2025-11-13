#pragma once

#include "peripheral.hpp"
#include "bus.hpp"
#include "gpio.hpp"

namespace wibot {

#ifdef HAL_SPI_MODULE_ENABLED

#define PWM_PER_DECL

//struct SpiConfig {
//    DataWidth dataWidth   : 3;
//    u8   dummyCycles : 4;
//    bool      autoDisable : 1 = true;
//    u32              : 24;
//};

class Spi : public SpiMaster, private PeripheralBase {
   public:
    explicit Spi(SPI_HandleTypeDef& handle, Pin* csPin);
    ~Spi();

    Result setConfig(SpiConfig& config) override;

   public:
    AsyncResult read(const Slice& data) override;
    AsyncResult write(const Slice& data) override;
    AsyncResult writeRead(const Slice& txData, const Slice& rxData) override;
    Result      begin() override;
    Result      end() override;

   private:
    SPI_HandleTypeDef* _handle;
    Pin*               _csPin;
    SpiConfig          _config;
#ifdef STM32H7xx
#if CHIP_SPI_READ_DMA_ENABLED
    Slice _rxUserBuffer;
#endif
#endif
    AsyncSource _asyncSource;

   protected:
    static void _onReadCplt(SPI_HandleTypeDef* handle);
    static void _onWriteCplt(SPI_HandleTypeDef* handle);
    static void _onError(SPI_HandleTypeDef* handle);
    static void _onWriteReadCplt(SPI_HandleTypeDef* handle);
};

#endif  // HAL_SPI_MODULE_ENABLED

}  // namespace wibot

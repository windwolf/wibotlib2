#pragma once

#include "pin.hpp"
#include "buffer.hpp"
#include "peripheral.hpp"
#include "wait-handler.hpp"
#include "bus.hpp"

namespace wibot {

#ifdef HAL_SPI_MODULE_ENABLED

#define PWM_PER_DECL

//struct SpiConfig {
//    DataWidth dataWidth   : 3;
//    u8   dummyCycles : 4;
//    bool      autoDisable : 1 = true;
//    u32              : 24;
//};

class Spi : public SpiMasterBus, private PeripheralBase, private Initializable {
   public:
    explicit Spi(SPI_HandleTypeDef* handle);
    Spi(SPI_HandleTypeDef* handle, Pin* cs);
    ~Spi();

    Result setConfig(SpiConfig& config) override;

   public:
    Result begin() override;
    Result end() override;

    Result read(void* data, u32 size, WaitHandler& waitHandler) override;
    Result write(void* data, u32 size, WaitHandler& waitHandler) override;
    Result writeRead(void* txData, void* rxData, u32 size, WaitHandler& waitHandler) override;

   private:
    void _init() override;

   private:
    SPI_HandleTypeDef* _handle;
    Pin*               _cs;
    SpiConfig          _config;
    WaitTrigger        _waitTrigger;

   protected:
    static void _onReadCplt(SPI_HandleTypeDef* handle);
    static void _onWriteCplt(SPI_HandleTypeDef* handle);
    static void _onError(SPI_HandleTypeDef* handle);
    static void _onWriteReadCplt(SPI_HandleTypeDef* handle);
};

#endif  // HAL_SPI_MODULE_ENABLED

}  // namespace wibot

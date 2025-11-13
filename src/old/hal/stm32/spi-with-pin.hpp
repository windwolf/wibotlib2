#pragma once

#include "pin.hpp"
#include "buffer.hpp"
#include "peripheral.hpp"
#include "wait-handler.hpp"
#include "hal-spi.hpp"

namespace wibot {

#ifdef HAL_SPI_MODULE_ENABLED

//class SpiWithPins : public Spi {
//   public:
//    SpiWithPins(SPI_HandleTypeDef* handle, Pin* cs, Pin* rw, Pin* dc);
//    ~SpiWithPins();
//    Result read(bool isData, void* data, u32 size, WaitHandler& waitHandler);
//    Result write(bool isData, void* data, u32 size, WaitHandler& waitHandler);
//    Result begin(WaitHandler& waitHandler);
//    Result end(WaitHandler& waitHandler);
//
//   private:
//    void _init() override;
//
//   public:
//    SpiWithPinsConfig pinConfig;
//
//   private:
//    SPI_HandleTypeDef* _handle;
//
//    Pin* _cs;
//    Pin* _rw;
//    Pin* _dc;
//    union {
//        u8 value;
//        struct {
//            u8 busy : 1;
//        };
//    } _pinStatus;
//
//    void        _setCs(bool isEnable);
//    void        _setDc(bool isData);
//    void        _setRw(bool isRead);
//    static void _onReadCplt(SPI_HandleTypeDef* handle);
//    static void _onWriteCplt(SPI_HandleTypeDef* handle);
//};

#endif  // HAL_SPI_MODULE_ENABLED

}  // namespace wibot

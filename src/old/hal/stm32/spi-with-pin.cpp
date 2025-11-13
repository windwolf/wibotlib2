#include "spi-with-pin.hpp"

#include "peripheral.hpp"

#ifdef HAL_SPI_MODULE_ENABLED

namespace wibot {

// SpiWithPins--------------------

//SpiWithPins::SpiWithPins(SPI_HandleTypeDef* handle, Pin* cs, Pin* rw, Pin* dc)
//    : Spi(handle), _handle(handle), _cs(cs), _rw(rw), _dc(dc){};
//
//void SpiWithPins::_init() {
//    if (_cs) {
//        _cs->config.inverse = !pinConfig.csPinHighIsEnable;
//    }
//    if (_dc) {
//        _dc->config.inverse = pinConfig.dcPinHighIsCmd;
//    }
//    if (_rw) {
//        _rw->config.inverse = pinConfig.rwPinHighIsWrite;
//    }
//    HAL_SPI_RegisterCallback(_handle, HAL_SPI_TX_COMPLETE_CB_ID, _onWriteCplt);
//    HAL_SPI_RegisterCallback(_handle, HAL_SPI_RX_COMPLETE_CB_ID, _onReadCplt);
//    HAL_SPI_RegisterCallback(_handle, HAL_SPI_ERROR_CB_ID, &_onError);
//};
//
//SpiWithPins::~SpiWithPins() {
//}
//
//Result SpiWithPins::read(bool isData, void* data, u32 size, WaitHandler& waitHandler) {
//    if (pinConfig.autoCs || _pinStatus.busy) {
//        _setCs(true);
//    }
//    _setRw(true);
//    _setDc(isData);
//    return Spi::read(data, size, waitHandler);
//};
//Result SpiWithPins::write(bool isData, void* data, u32 size, WaitHandler& waitHandler) {
//    if (pinConfig.autoCs || _pinStatus.busy) {
//        _setCs(true);
//    }
//
//    _setRw(false);
//    _setDc(isData);
//    return Spi::write(data, size, waitHandler);
//};
//
//Result SpiWithPins::begin(WaitHandler& waitHandler) {
//    _setCs(true);
//    if (_pinStatus.busy) {
//        return Result::kBusy;
//    }
//
//    _pinStatus.busy = 1;
//    waitHandler.setDone(this);
//    return Result::kOk;
//};
//Result SpiWithPins::end(WaitHandler& waitHandler) {
//    _pinStatus.busy = 0;
//    _setCs(false);
//    waitHandler.setDone(this);
//    return Result::kOk;
//};
//
//void SpiWithPins::_onReadCplt(SPI_HandleTypeDef* handle) {
//    SpiWithPins* spi = (SpiWithPins*)PeripheralManager::getInstance().getPeripheral(handle);
//    if (spi->pinConfig.autoCs || !spi->_pinStatus.busy) {
//        spi->_setCs(false);
//    }
//    Spi::_onReadCplt(handle);
//};
//void SpiWithPins::_onWriteCplt(SPI_HandleTypeDef* handle) {
//    SpiWithPins* spi = (SpiWithPins*)PeripheralManager::getInstance().getPeripheral(handle);
//    if (spi->pinConfig.autoCs || !spi->_pinStatus.busy) {
//        spi->_setCs(false);
//    }
//    Spi::_onWriteCplt(handle);
//};
//
//void SpiWithPins::_setCs(bool isEnable) {
//    if (_cs != NULL) _cs->write(isEnable ? PinStatus::kSet : PinStatus::kReset);
//};
//
//void SpiWithPins::_setDc(bool isData) {
//    if (_dc != NULL) _dc->write(isData ? PinStatus::kSet : PinStatus::kReset);
//};
//
//void SpiWithPins::_setRw(bool isRead) {
//    if (_rw != NULL) _rw->write(isRead ? PinStatus::kSet : PinStatus::kReset);
//}

};  // namespace wibot

#endif

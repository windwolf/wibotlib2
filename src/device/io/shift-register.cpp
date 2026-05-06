#include "shift-register.hpp"
#include "hal/system.hpp"

namespace wibot {
SinPoutShiftRegister::SinPoutShiftRegister(SpiMaster& spi, Pin& stcpPin)
    : _spi(spi), _stcpPin(&stcpPin) {
    _stcpPin->setValue(false);
};

Result SinPoutShiftRegister::write(Slice data) {
    Result rst;
    rst = _spi.begin();
    if (rst != Result::kOk) {
        return rst;
    }
    // 通过SPI写入数据
    rst = _spi.write(data).wait(TIMEOUT_FOREVER);
    _spi.end();
    if (rst != Result::kOk) {
        return rst;
    }
    System::delayUs(1);
    _stcpPin->setValue(true);
    System::delayUs(1);  // 确保数据锁存
    _stcpPin->setValue(false);

    return Result::kOk;
};

PinSoutShiftRegister::PinSoutShiftRegister(SpiMaster& spi, Pin& plPin) : _spi(spi), _plPin(&plPin) {
    _plPin->setValue(true);
};

Result PinSoutShiftRegister::read(const Slice& data) {
    Result rst;
    _plPin->setValue(false);
    System::delayUs(1);  // 确保数据准备好
    _plPin->setValue(true);
    System::delayUs(1);
    rst = _spi.begin();
    if (rst != Result::kOk) {
        return rst;
    }
    // 通过SPI读取数据
    rst = _spi.read(data).wait(TIMEOUT_FOREVER);
    _spi.end();
    if (rst != Result::kOk) {
        return rst;
    }

    return Result::kOk;
};

}  // namespace wibot

#include "shift-register.hpp"
#include "hal/system.hpp"

namespace wibot::device {
SinPoutShiftRegister::SinPoutShiftRegister(SpiMaster& spi, hal::Pin& stcpPin)
    : _spi(spi), _stcpPin(&stcpPin) {
    _stcpPin->setValue(false);
};

Result SinPoutShiftRegister::write(Slice data) {
    Result rst;
    _spi.begin();
    // 通过SPI写入数据
    rst = _spi.write(data).wait(TIMEOUT_FOREVER);
    _spi.end();
    if (rst != Result::kOk) {
        return rst;
    }
    hal::System::delayUs(1);
    _stcpPin->setValue(true);
    hal::System::delayUs(1);  // 确保数据锁存
    _stcpPin->setValue(false);

    return Result::kOk;
};

PinSoutShiftRegister::PinSoutShiftRegister(SpiMaster& spi, hal::Pin& plPin)
    : _spi(spi), _plPin(&plPin) {
    _plPin->setValue(true);
};

Result PinSoutShiftRegister::read(const Slice& data) {
    Result rst;
    _plPin->setValue(false);
    hal::System::delayUs(1);  // 确保数据准备好
    _plPin->setValue(true);
    hal::System::delayUs(1);
    _spi.begin();
    // 通过SPI读取数据
    rst = _spi.writeRead(Slice(nullptr, data.size), data).wait(TIMEOUT_FOREVER);
    _spi.end();
    if (rst != Result::kOk) {
        return rst;
    }

    return Result::kOk;
};

}  // namespace wibot::device
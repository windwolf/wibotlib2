#include "Tm1652.hpp"
namespace wibot::device {
Tm1652::~Tm1652() {
}

/**
 * @brief 
 * 
 * @param brightness grid brightness: 0-15
 * @param current sig driver current: 0-7
 * @param mode  0x00: 8sig/5grid, 0x01: 7sig/6grid
 * @param waitHandler 
 * @return Result 
 */
os::AsyncResult Tm1652::setup(u8 brightness, u8 current, u8 mode) {
    _cmdBuf[0] = 0x18;
    _cmdBuf[1] =
        (brightness & 0x0F) << 4 | (current & 0x07) << 1 | (mode & 0x01);  // Example command format
    return _uart.write(Slice(_cmdBuf, 2));
}

os::AsyncResult Tm1652::updateDisplay(u8* data, u8 length, u8 startGrid) {
    if (length > 6 || length == 0 || startGrid > 7) {
        return os::AsyncResult::fromError(Result::kInvalidParameter);
    }

    _cmdBuf[0] = ((startGrid & 0x07) << 5) | 0x08;
    _cmdBuf[1] = data[4];
    _cmdBuf[2] = data[3];
    _cmdBuf[3] = data[2];
    _cmdBuf[4] = data[1];
    _cmdBuf[5] = data[0];

    return _uart.write(Slice(_cmdBuf, length + 1));
}

os::AsyncResult Tm1652::updateDisplay(u8* data, u8 length) {
    return updateDisplay(data, length, 0);
}
}  // namespace wibot::device

#include "Tm1652.hpp"
namespace wibot {
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
AsyncResult Tm1652::setup(u8 brightness, u8 current, u8 mode) {
    _cmdBuf[0] = 0x18;
    _cmdBuf[1] =
        (brightness & 0x0F) << 4 | (current & 0x07) << 1 | (mode & 0x01);  // Example command format
    return _uart.write(Slice(_cmdBuf, 2));
}

AsyncResult Tm1652::updateDisplay(u8* data, u8 length, u8 startGrid) {
    if (data == nullptr || length > 6 || length == 0 || startGrid > 7 ||
        startGrid + length > 8) {
        return AsyncResult::fromError(Result::kInvalidParameter);
    }

    _cmdBuf[0] = ((startGrid & 0x07) << 5) | 0x08;
    for (u8 i = 0; i < length; ++i) {
        _cmdBuf[i + 1] = data[length - 1 - i];
    }

    return _uart.write(Slice(_cmdBuf, length + 1));
}

AsyncResult Tm1652::updateDisplay(u8* data, u8 length) {
    return updateDisplay(data, length, 0);
}
}  // namespace wibot

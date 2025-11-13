#include "soft-i2c.hpp"
#include "async.hpp"
#include "system.hpp"

namespace wibot {

#define i2c_sda_in() LL_GPIO_SetOutputPin(_sdaPort, _sdaPin);

#define i2c_sda_out()

#define i2c_scl_in() LL_GPIO_SetOutputPin(_sclPort, _sclPin);

#define i2c_scl_out()

#define i2c_sda_lo() LL_GPIO_ResetOutputPin(_sdaPort, _sdaPin);
#define i2c_sda_hi() LL_GPIO_SetOutputPin(_sdaPort, _sdaPin);

#define i2c_scl_lo() LL_GPIO_ResetOutputPin(_sclPort, _sclPin);
#define i2c_scl_hi() LL_GPIO_SetOutputPin(_sclPort, _sclPin);

#define i2c_sda_read() LL_GPIO_IsInputPinSet(_sdaPort, _sdaPin)
#define i2c_scl_read() LL_GPIO_IsInputPinSet(_sclPort, _sclPin)

#define i2c_delay() \
    if (_i2cDelay != 0) System::delayUs(_i2cDelay);

SoftI2cMaster::SoftI2cMaster(GPIO_TypeDef *sclPort, u32 sclPin, GPIO_TypeDef *sdaPort, u32 sdaPin)
    : _sclPort(sclPort), _sclPin(sclPin), _sdaPort(sdaPort), _sdaPin(sdaPin) {
    I2cTimingConfig cfg = {.frequency = 100000, .timeout = 1000, .stretch = false};
    setTimingConfig(cfg);
    LL_GPIO_SetPinMode(_sclPort, _sclPin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinPull(_sclPort, _sclPin, LL_GPIO_PULL_UP);
    LL_GPIO_SetPinMode(_sdaPort, _sdaPin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinPull(_sdaPort, _sdaPin, LL_GPIO_PULL_UP);

    //    i2c_scl_hi();
    //
    //    if (_i2cDelay != 0) System::delayUs(_i2cDelay);
    //
    //    i2c_sda_hi();
    //
    //    for (u8 i = 0; i < 4; i++)  // 4 times the normal delay, to claim the bus.
    //    {
    //        if (_i2cDelay != 0) System::delayUs(_i2cDelay);
    //    }
    //    os::sleep(2);  // 1ms didn't always work.
}

/**
 * SCL: ~¯¯\_
 * SDA: ~¯\__
 * @return
 */
Result SoftI2cMaster::_i2cStart() {
    i2c_sda_out();
    i2c_sda_hi();  // can perhaps be removed some day ? if the rest of the code is okay

    i2c_delay();

    i2c_scl_out();
    i2c_scl_hi();  // can perhaps be removed some day ? if the rest of the code is okay

    i2c_delay();

    // Both the sda and scl should be high.
    // If not, there might be a hardware problem with the i2c bus signal lines.
    // This check was added to prevent that a shortcut of sda would be seen as a valid ACK
    // from a i2c Slave.

    if (i2c_sda_read() == 0 || i2c_scl_read() == 0) {
        return Result::kError;
    } else {
        i2c_sda_lo();

        i2c_delay();

        i2c_scl_lo();

        // if (_i2cDelay != 0) System::delayUs(_i2cDelay);
    }
    return Result::kOk;
}

void SoftI2cMaster::_i2cRestart() {
    i2c_sda_out();

    i2c_delay();

    i2c_sda_hi();

    i2c_delay();

    i2c_scl_hi();

    i2c_delay();

    i2c_sda_lo();  // force SCL low

    i2c_delay();

    i2c_scl_lo();
}
void SoftI2cMaster::_i2cStop() {
    i2c_sda_out();
    i2c_scl_out();

    i2c_scl_lo();  // ADDED1, it should already be low.

    i2c_delay();

    i2c_sda_lo();

    i2c_delay();

    // For a stop, make SCL high wile SDA is still low
    i2c_scl_hi();

    // Check if clock stretching by the Slave should be detected.
    if (_baseConfig.stretch) {
        // Wait until the clock is high, the Slave could keep it low for clock stretching.
        // Clock pulse stretching during a stop condition seems odd, but when
        // the Slave is an Arduino, it might happen.
        unsigned long prevMillis = System::getTickUs();
        //i2c_scl_in();
        while (i2c_scl_read() == 0) {
            if (System::getTickUs() - prevMillis >= _baseConfig.timeout) break;
        };
        //i2c_scl_out();
    }

    i2c_delay();

    // complete the STOP by setting SDA high
    i2c_sda_hi();

    // A delay after the STOP for safety.
    // It is not known how fast the next START will happen.
    i2c_delay();
    //
    //    i2c_sda_in();
    //    i2c_scl_in();
}

/**
 * SCL: ___/¯¯¯\
 * SDA: -X------
 */
void SoftI2cMaster::_i2cWriteBit(u8 c) {
    i2c_delay();

    if (c == 0) {
        i2c_sda_lo();
    } else {
        i2c_sda_hi();
    }

    i2c_delay();

    i2c_scl_hi();  // clock high: the Slave will read the sda signal

    // Check if clock stretching by the Slave should be detected.
    if (_baseConfig.stretch) {
        // If the Slave was stretching the clock pulse, the clock would not go high immediately.
        // For example if the Slave is an Arduino, that has other interrupts running (for example Serial data).
        unsigned long prevMillis = System::getTickUs();
        //i2c_scl_in();
        while (i2c_scl_read() == 0) {
            if (System::getTickUs() - prevMillis >= _baseConfig.timeout) break;
        };
        //i2c_scl_out();
    }

    i2c_delay();

    i2c_scl_lo();

    // if (_i2cDelay != 0) System::delayUs(_i2cDelay);
}

/**
 * SCL: ___/¯¯¯\
 * SDA: ~~~~~↓~~
 * @return
 */
u8 SoftI2cMaster::_i2cReadBit() {
    i2c_delay();

    i2c_scl_hi();

    // Check if clock stretching by the Slave should be detected.
    if (_baseConfig.stretch) {
        // Wait until the clock is high, the Slave could keep it low for clock stretching.
        unsigned long prevMillis = System::getTickUs();
        //i2c_scl_in();
        while (i2c_scl_read() == 0) {
            if (System::getTickUs() - prevMillis >= _baseConfig.timeout) break;
        };
        //i2c_scl_out();
    }

    // After the clock stretching, this delay has still be done before reading sda.
    if (_i2cDelay != 0) System::delayUs(_i2cDelay);

    // i2c_sda_in();
    u8 c = i2c_sda_read();

    i2c_scl_lo();

    // if (_i2cDelay != 0) System::delayUs(_i2cDelay);

    return (c);
}
Result SoftI2cMaster::_i2cWrite(u8 c) {
    i2c_sda_out();
    for (u8 i = 0; i < 8; i++) {
        _i2cWriteBit(c & 0x80);  // highest bit first
        c <<= 1;
    }
    i2c_sda_in();
    if (_i2cDelay != 0) System::delayUs(_i2cDelay);
    return _i2cReadBit() == 0 ? Result::kOk : Result::kError;
}

u8 SoftI2cMaster::_i2cRead(bool ack) {
    i2c_sda_in();
    u8 res = 0;

    for (u8 i = 0; i < 8; i++) {
        res <<= 1;
        res |= _i2cReadBit();
    }

    i2c_sda_out();
    if (ack) {
        _i2cWriteBit(0);
    } else {
        _i2cWriteBit(1);
    }

    if (_i2cDelay != 0) System::delayUs(_i2cDelay);

    return (res);
}

SoftI2cMaster::~SoftI2cMaster() {
    _i2cStop();
}

Result SoftI2cMaster::setTimingConfig(I2cTimingConfig &config) {
    ASSERT(config.frequency <= 400000, "i2c frequency is too high");
    _baseConfig = config;

    _i2cDelay = 1000000 / config.frequency / 2;
    return Result::kOk;
}
Result SoftI2cMaster::setTransitionConfig(I2cMasterTransitionConfig &config) {
    _transitionConfig = config;
    return Result::kOk;
}

AsyncResult SoftI2cMaster::readReg(u16 regAddr, const Slice &data) {
    Result rst = Result::kOk;

    do {
        rst = _i2cStart();
        if (rst != Result::kOk) break;

        rst = _i2cSendAddress(false);
        if (rst != Result::kOk) break;

        if (_transitionConfig.dataWidth == DataWidth::k8Bits) {
            rst = _i2cWrite(regAddr);
            if (rst != Result::kOk) break;
        } else if (_transitionConfig.dataWidth == DataWidth::k16Bits) {
            rst = _i2cWrite(regAddr >> 8);
            if (rst != Result::kOk) break;
            rst = _i2cWrite(regAddr & 0xFF);
            if (rst != Result::kOk) break;
        } else {
            rst = Result::kNotSupport;
            break;
        }

        _i2cRestart();

        rst = _i2cSendAddress(true);
        if (rst != Result::kOk) break;

        auto pData = data.data;
        auto len   = data.size;  // size is already in bytes
        for (u16 i = 0; i < len; i++) {
            pData[i] = _i2cRead(i != (len - 1));
        }

    } while (false);
    _i2cStop();

    return AsyncResult::fromResult(rst);
}

AsyncResult SoftI2cMaster::writeReg(u16 regAddr, const Slice &data) {
    Result rst = Result::kOk;
    do {
        rst = _i2cStart();
        if (rst != Result::kOk) break;

        rst = _i2cSendAddress(false);
        if (rst != Result::kOk) break;

        if (_transitionConfig.dataWidth == DataWidth::k8Bits) {
            rst = _i2cWrite(regAddr);
            if (rst != Result::kOk) break;
        } else if (_transitionConfig.dataWidth == DataWidth::k16Bits) {
            rst = _i2cWrite(regAddr >> 8);
            if (rst != Result::kOk) break;
            rst = _i2cWrite(regAddr & 0xFF);
            if (rst != Result::kOk) break;
        } else {
            rst = Result::kNotSupport;
            break;
        }

        auto pData = data.data;
        auto len   = data.size;  // size is already in bytes
        for (u16 i = 0; i < len; i++) {
            rst = _i2cWrite(pData[i]);
            if (rst != Result::kOk) break;
        }

    } while (false);
    _i2cStop();

    return AsyncResult::fromResult(rst);
}

Result SoftI2cMaster::_i2cSendAddress(bool isRead) {
    ASSERT(_transitionConfig.deviceAddr != 0, "i2c device address is not set");

    if (!_transitionConfig.is10BitsAddr) {
        if (_i2cWrite((_transitionConfig.deviceAddr << 1) | isRead) != Result::kOk) {
            return Result::kNoResource;
        }
    } else {
        if (_i2cWrite(0xF0 | ((_transitionConfig.deviceAddr >> 7) & 0x06) | isRead) !=
            Result::kOk) {
            return Result::kNoResource;
        }
        if (_i2cWrite(_transitionConfig.deviceAddr & 0xFF) != Result::kOk) {
            return Result::kNoResource;
        }
    }
    return Result::kOk;
}
AsyncResult SoftI2cMaster::read(const Slice &data) {
    Result rst = Result::kOk;
    do {
        rst = _i2cStart();
        if (rst != Result::kOk) break;

        rst = _i2cSendAddress(true);
        if (rst != Result::kOk) break;

        auto len = data.size;  // size is already in bytes
        for (u32 i = 0; i < len; i++) {
            data.data[i] = _i2cRead(i != len - 1);
        }
    } while (false);
    _i2cStop();

    return AsyncResult::fromResult(rst);
}
AsyncResult SoftI2cMaster::write(const Slice &data) {
    Result rst = Result::kOk;
    do {
        rst = _i2cStart();
        if (rst != Result::kOk) break;

        rst = _i2cSendAddress(false);
        if (rst != Result::kOk) break;

        auto len = data.size;  // size is already in bytes
        for (u32 i = 0; i < len; i++) {
            rst = _i2cWrite(data.data[i]);
            if (rst != Result::kOk) break;
        }
    } while (false);
    _i2cStop();

    return AsyncResult::fromResult(rst);
}
}  // namespace wibot

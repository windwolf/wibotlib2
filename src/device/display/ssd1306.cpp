#include "ssd1306.hpp"

#include "os.hpp"

#define SSD1306_DATA_STREAM    0x40
#define SSD1306_DATA_SINGLE    0xC0
#define SSD1306_COMMAND_STREAM 0x00
#define SSD1306_COMMAND_SINGLE 0x80

namespace wibot {

Ssd1306::Ssd1306(I2cMaster* i2c) : _i2c(i2c) {};

void Ssd1306::_sendCommand(u8 cmdSize) {
    if (cmdSize == 1) {
        auto ar = _i2c->writeReg(SSD1306_COMMAND_SINGLE, Slice(_cmdBuffer, cmdSize));
        ar.wait(TIMEOUT_FOREVER);
    } else {
        auto ar = _i2c->writeReg(SSD1306_COMMAND_STREAM, Slice(_cmdBuffer, cmdSize));
        ar.wait(TIMEOUT_FOREVER);
    }
};

void Ssd1306::_sendData(const Slice& data) {
    if (data.size == 1) {
        auto ar = _i2c->writeReg(SSD1306_DATA_SINGLE, data);
        ar.wait(TIMEOUT_FOREVER);
    } else {
        auto ar = _i2c->writeReg(SSD1306_DATA_STREAM, data);
        ar.wait(TIMEOUT_FOREVER);
    }
};

void Ssd1306::_setMemMode() {
    _cmdBuffer[0] = SSD1306_CMD_SET_MEMORY_ADDRESSING_MODE;
    _cmdBuffer[1] = (u8)config.memoryMode;
    _sendCommand(2);
};

void Ssd1306::display(bool on) {
    if (on) {
        if (config.enableChargePump) {
            _cmdBuffer[0] = SSD1306_CMD_SET_CHARGE_PUMP;
            _cmdBuffer[1] = 0x14;
            _sendCommand(2);
        } else {
            _cmdBuffer[0] = SSD1306_CMD_SET_CHARGE_PUMP;
            _cmdBuffer[1] = 0x04;
            _sendCommand(2);
        }
        _cmdBuffer[0] = SSD1306_CMD_DISPLAY_ON;
        _sendCommand(1);
    } else {
        _cmdBuffer[0] = SSD1306_CMD_DISPLAY_OFF;
        _sendCommand(1);
        _cmdBuffer[0] = SSD1306_CMD_SET_CHARGE_PUMP;
        _cmdBuffer[1] = 0x04;
        _sendCommand(2);
    }
};

void Ssd1306::setContrast(u8 contrast) {
    _cmdBuffer[0] = SSD1306_CMD_CONSTRAST_CONTROL;
    _cmdBuffer[1] = contrast;
    _sendCommand(2);
};

void Ssd1306::setPos(u8 page, u8 column) {
    if (config.memoryMode == kPAGE) {
        _cmdBuffer[0] = SSD1306_CMD_SET_PAGE_START_ADDRESS | page;
        _sendCommand(1);
        _cmdBuffer[0] = SSD1306_CMD_SET_COLUMN_START_ADDRESS_LOWER | (column & 0x0F);
        _sendCommand(1);
        _cmdBuffer[0] = SSD1306_CMD_SET_COLUMN_START_ADDRESS_HIGHER | (column >> 4);
        _sendCommand(1);
    } else {
        _cmdBuffer[0] = SSD1306_CMD_SET_PAGE_ADDRESS;
        _cmdBuffer[1] = page;
        _cmdBuffer[2] = (config.height - 1) / 8;
        _sendCommand(3);
        _cmdBuffer[0] = SSD1306_CMD_SET_COLUMN_ADDRESS;
        _cmdBuffer[1] = column;
        _cmdBuffer[2] = config.width - 1;
        _sendCommand(3);
    }
};

void Ssd1306::clear() {
    for (u16 i = 0; i < bufferSize; i++) {
        dataBuffer[i] = 0x00;
    }

    Ssd1306MemoryAddressingMode oldMode = config.memoryMode;
    config.memoryMode                   = kHORIZONTAL;
    _setMemMode();

    setPos(0, 0);

    _sendData(Slice(dataBuffer, bufferSize));

    config.memoryMode = oldMode;
    _setMemMode();
};

// 初始化SSD1306
void Ssd1306::init() {
    _i2c->setTransitionConfig(0x78 >> 1);

    bufferSize = config.width * config.height / 8;
    os::sleep(100);

    display(false);

    _setMemMode();

    _cmdBuffer[0] = SSD1306_CMD_SET_DISPLAY_START_LINE | config.displayStartLine;
    _sendCommand(1);

    if (config.comInverted) {
        _cmdBuffer[0] = SSD1306_CMD_SET_COM_OUTPUT_SCAN_DIRECTION_REMAP;
    } else {
        _cmdBuffer[0] = SSD1306_CMD_SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL;
    }
    _sendCommand(1);

    if (config.segmentInverted) {
        _cmdBuffer[0] = SSD1306_CMD_SET_SEGMENT_REMAP_INVERSE;
    } else {
        _cmdBuffer[0] = SSD1306_CMD_SET_SEGMENT_REMAP_NORMAL;
    }
    _sendCommand(1);

    if (config.displayInverted) {
        _cmdBuffer[0] = SSD1306_CMD_DISPLAY_INVERTED;
    } else {
        _cmdBuffer[0] = SSD1306_CMD_DISPLAY_NORMAL;
    }
    _sendCommand(1);

    _cmdBuffer[0] = SSD1306_CMD_SET_MULTIPLEX_RATIO;
    _cmdBuffer[1] = config.multiplexRatio;
    _sendCommand(2);

    _cmdBuffer[0] = SSD1306_CMD_SET_DISPLAY_OFFSET;
    _cmdBuffer[1] = config.displayOffset;
    _sendCommand(2);

    _cmdBuffer[0] = SSD1306_CMD_SET_DISPLAY_CLOCK_DIVIDE_RATIO;
    _cmdBuffer[1] = (u8)((config.fosc << 4) | config.clkDivide);
    _sendCommand(2);

    _cmdBuffer[0] = SSD1306_CMD_SET_PRECHARGE_PERIOD;
    _cmdBuffer[1] = config.phase1period | (config.phase2period << 4);
    _sendCommand(2);

    _cmdBuffer[0] = SSD1306_CMD_SET_COM_PINS_CONFIGURATION;
    _cmdBuffer[1] = 0x02 | (config.comLeftRightRemap << 5) | (config.comAlternative << 4);
    _sendCommand(2);

    _cmdBuffer[0] = SSD1306_CMD_SET_VCOMH_DESELECT_LEVEL;
    _cmdBuffer[1] = config.vcomhDeselectLevel;
    _sendCommand(2);

    _cmdBuffer[0] = SSD1306_CMD_ENTIRE_DISPLAY_ON;
    _sendCommand(1);

    clear();

    os::sleep(100);

    display(true);

    os::sleep(100);
};

void Ssd1306::draw() {
    setPos(0, 0);
    _sendData(Slice(dataBuffer, bufferSize));
}
}  // namespace wibot

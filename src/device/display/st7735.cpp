#include "st7735.hpp"

namespace wibot {
#define ST7735_INTERNAL_BUFFER_SIZE (16)

Result ST7735::init() {
    _pvGamma[0]  = 0x02U;
    _pvGamma[1]  = 0x1CU;
    _pvGamma[2]  = 0x07U;
    _pvGamma[3]  = 0x12U;
    _pvGamma[4]  = 0x37U;
    _pvGamma[5]  = 0x32U;
    _pvGamma[6]  = 0x29U;
    _pvGamma[7]  = 0x2DU;
    _pvGamma[8]  = 0x29U;
    _pvGamma[9]  = 0x25U;
    _pvGamma[10] = 0x2BU;
    _pvGamma[11] = 0x39U;
    _pvGamma[12] = 0x00U;
    _pvGamma[13] = 0x01U;
    _pvGamma[14] = 0x03U;
    _pvGamma[15] = 0x10U;

    _nvGamma[0]  = 0x03U;
    _nvGamma[1]  = 0x1DU;
    _nvGamma[2]  = 0x07U;
    _nvGamma[3]  = 0x06U;
    _nvGamma[4]  = 0x2EU;
    _nvGamma[5]  = 0x2CU;
    _nvGamma[6]  = 0x29U;
    _nvGamma[7]  = 0x2DU;
    _nvGamma[8]  = 0x2EU;
    _nvGamma[9]  = 0x2EU;
    _nvGamma[10] = 0x37U;
    _nvGamma[11] = 0x3FU;
    _nvGamma[12] = 0x00U;
    _nvGamma[13] = 0x00U;
    _nvGamma[14] = 0x02U;
    _nvGamma[15] = 0x10U;
    return Result::kOk;
};
Result ST7735::reset() {
    St77xx::sendCommand(ST7735_CMD_SOFTWARE_RESET);
    os::sleep(120);

    sendCommand(ST7735_CMD_SOFTWARE_RESET);
    os::sleep(120);

    sendCommand(ST7735_CMD_SLEEP_OUT);

    static const u8 frctl1[3] = {0x01U, 0x2CU, 0x2DU};
    sendWriteCommand(ST7735_CMD_FRAME_RATE_CTRL1, Slice((u8 *)(frctl1), 3));

    static const u8 frctl2[3] = {0x01U, 0x2CU, 0x2DU};
    sendWriteCommand(ST7735_CMD_FRAME_RATE_CTRL2, Slice((u8 *)(frctl2), 3));

    static const u8 frctl3[6] = {0x01U, 0x2CU, 0x2DU, 0x01U, 0x2CU, 0x2DU};
    sendWriteCommand(ST7735_CMD_FRAME_RATE_CTRL3, Slice((u8 *)(frctl3), 6));

    static const u8 fictl[1] = {0x07U};
    sendWriteCommand(ST7735_CMD_FRAME_INVERSION_CTRL, Slice((u8 *)(fictl), 1));

    static const u8 pctl1[3] = {0xA2U, 0x02U, 0x84U};
    sendWriteCommand(ST7735_CMD_PWR_CTRL1, Slice((u8 *)(pctl1), 3));

    static const u8 pctl2[1] = {0xC5U};
    sendWriteCommand(ST7735_CMD_PWR_CTRL2, Slice((u8 *)(pctl2), 1));

    static const u8 pctl3[2] = {0x0AU, 0x00U};
    sendWriteCommand(ST7735_CMD_PWR_CTRL3, Slice((u8 *)(pctl3), 2));

    static const u8 pctl4[2] = {0x8AU, 0x2AU};
    sendWriteCommand(ST7735_CMD_PWR_CTRL4, Slice((u8 *)(pctl4), 2));

    static const u8 pctl5[2] = {0x8AU, 0xEEU};
    sendWriteCommand(ST7735_CMD_PWR_CTRL5, Slice((u8 *)(pctl5), 2));

    static const u8 vcomctl1[1] = {0x0EU};
    sendWriteCommand(ST7735_CMD_VCOMH_VCOML_CTRL1, Slice((u8 *)(vcomctl1), 1));

    sendCommand(ST7735_CMD_DISPLAY_INVERSION_ON);

    sendWriteCommand(ST7735_CMD_INTERFACE_PIXEL_FORMAT, Slice(&config.colorMode, 1));

    sendWriteCommand(ST7735_CMD_POSITIVE_VOLTAGE_GAMMA_CONTROL, Slice(_pvGamma, 16));

    sendWriteCommand(ST7735_CMD_NAGATIVE_VALTAGE_GAMMA_CONTROL, Slice(_nvGamma, 16));

    sendCommand(ST7735_CMD_NORMAL_DISPLAY_MODE_ON);

    sendCommand(ST7735_CMD_DISPLAY_ON);

    sendWriteCommand(ST7735_CMD_MEMORY_DATA_ACCESS_CONTROL, Slice(&config.orientation, 1));

    return Result::kOk;
};
Result ST7735::inversion(bool on) {
    sendCommand((on ? ST7735_CMD_DISPLAY_INVERSION_ON : ST7735_CMD_DISPLAY_INVERSION_OFF));

    return Result::kOk;
};
Result ST7735::sleep(bool on) {
    sendCommand((on ? ST7735_CMD_SLEEP_IN : ST7735_CMD_SLEEP_OUT));

    return Result::kOk;
};

Result ST7735::setDisplayWindow(u16 x1, u16 y1, u16 x2, u16 y2) {
    x1 += config.xOffset;
    x2 += config.xOffset;
    y1 += config.yOffset;
    y2 += config.yOffset;

    u16 x[2] = {x1, x2};
    sendWriteCommand(ST7735_CMD_COLUMN_ADDRESS_SET, Slice((u8 *)x, 4));

    u16 y[2] = {y1, y2};
    sendWriteCommand(ST7735_CMD_ROW_ADDRESS_SET, Slice((u8 *)y, 4));
    return Result::kOk;
};
Result ST7735::setCursor(u16 x, u16 y) {
    x += config.xOffset;
    sendWriteCommand(ST7735_CMD_COLUMN_ADDRESS_SET, Slice((u8 *)&x, 2));
    y += config.yOffset;
    sendWriteCommand(ST7735_CMD_ROW_ADDRESS_SET, Slice((u8 *)&y, 2));

    return Result::kOk;
};

Result ST7735::display(bool on) {
    sendCommand((on ? ST7735_CMD_DISPLAY_ON : ST7735_CMD_DISPLAY_OFF));

    return Result::kOk;
};
Result ST7735::drawPixel(u16 x, u16 y, u16 color) {
    Result ret = Result::kOk;

    if ((x >= config.width) || (y >= config.height)) {
        return Result::kInvalidParameter;
    }
    ret = setCursor(x, y);

    if (ret != Result::kOk) {
        return ret;
    }

    sendWriteCommand(ST7735_CMD_MEMORY_WRITE, Slice((u8 *)&color, 4));

    return Result::kOk;
};
Result ST7735::drawHline(u16 x1, u16 y, u16 x2, u16 *data) {
    Result ret = Result::kOk;

    if ((x1 > config.width) || (x2 > config.width)) {
        return Result::kInvalidParameter;
    }

    ret = setCursor(x1, y);

    if (ret != Result::kOk) {
        return ret;
    }

    sendWriteCommand(ST7735_CMD_MEMORY_WRITE, Slice((u8 *)data, (x2 - x1 + 1) * 2));

    return ret;
};
Result ST7735::drawVline(u16 x, u16 y1, u16 y2, u16 *data) {
    Result ret = Result::kOk;

    if ((y1 > config.height) || (y2 > config.height)) {
        return Result::kInvalidParameter;
    }

    ret = setDisplayWindow(x, y1, x, y2);

    if (ret != Result::kOk) {
        return ret;
    }

    sendWriteCommand(ST7735_CMD_MEMORY_WRITE, Slice((u8 *)data, (y2 - y1 + 1) * 2));

    return ret;
};

Result ST7735::drawRect(u16 x1, u16 y1, u16 x2, u16 y2, u16 *data) {
    Result ret = Result::kOk;

    if (x1 > config.width || x2 > config.width) {
        return Result::kInvalidParameter;
    }
    if (y1 > config.height || y2 > config.height) {
        return Result::kInvalidParameter;
    }

    ret = setDisplayWindow(x1, y1, x2, y2);

    if (ret != Result::kOk) {
        return ret;
    }

    u32 size = (x2 - x1 + 1) * (y2 - y1 + 1);
    sendWriteCommand(ST7735_CMD_MEMORY_WRITE, Slice((u8 *)(data), size * 2));
    // st77xx_command_write_8(ST7735_CMD_MEMORY_WRITE, buf, size * 2);
    return ret;
};
Result ST7735::fillRect(u16 x1, u16 y1, u16 x2, u16 y2, u16 color) {
    Result ret = Result::kOk;

    if (x1 > config.width || x2 > config.width) {
        return Result::kInvalidParameter;
    }
    if (y1 > config.height || y2 > config.height) {
        return Result::kInvalidParameter;
    }

    ret = setDisplayWindow(x1, y1, x2, y2);

    if (ret != Result::kOk) {
        return ret;
    }

    u16 buf[ST7735_INTERNAL_BUFFER_SIZE];
    for (u32 i = 0; i < ST7735_INTERNAL_BUFFER_SIZE; i++) {
        buf[i] = color;
    }
    u32 size = (x2 - x1 + 1) * (y2 - y1 + 1);

    for (u32 i = 0; i < size / 8; i++) {
        sendWriteCommand(ST7735_CMD_MEMORY_WRITE, Slice((u8 *)buf, size * 2));
    }

    // sendWriteCommand(ST7735_CMD_MEMORY_WRITE, Slice((u8 *)buf, (size % 8) / 2),
    //                  DataWidth::k16Bits);

    return ret;
};
Result ST7735::drawBitmap(u32 x, u32 y, u8 *pBmp) {
    u32 index, size, width, height, y_pos;
    u8 *pbmp;

    /* Get bitmap data address offset */
    index = (u32)pBmp[10] + ((u32)pBmp[11] << 8) + ((u32)pBmp[12] << 16) + ((u32)pBmp[13] << 24);

    /* Read bitmap width */
    width = (u32)pBmp[18] + ((u32)pBmp[19] << 8) + ((u32)pBmp[20] << 16) + ((u32)pBmp[21] << 24);

    /* Read bitmap height */
    height = (u32)pBmp[22] + ((u32)pBmp[23] << 8) + ((u32)pBmp[24] << 16) + ((u32)pBmp[25] << 24);

    /* Read bitmap size */
    size = (u32)pBmp[2] + ((u32)pBmp[3] << 8) + ((u32)pBmp[4] << 16) + ((u32)pBmp[5] << 24);
    size = size - index;

    pbmp = pBmp + index;

    /* Remap Ypos, st7735 works with inverted X in case of bitmap */
    /* X = 0, cursor is on Top corner */
    y_pos = height - y - height;

    setDisplayWindow(x, y_pos, width, height);

    // u16 *buf = (u16 *)(buffer);
    Slice buf(pbmp, size);
    sendWriteCommand(ST7735_CMD_MEMORY_WRITE, buf);

    return Result::kOk;
};

Result ST7735::readId(u32 *id) {
    u32   id_temp = 0;
    u8    rd;
    Slice buf(&rd, 1);
    sendCommandData(ST7735_CMD_READ_ID1, buf, false);
    id_temp += rd;

    sendCommandData(ST7735_CMD_READ_ID2, buf, false);
    id_temp <<= 8;
    id_temp += rd;

    sendCommandData(ST7735_CMD_READ_ID3, buf, false);
    id_temp <<= 8;
    id_temp += rd;

    *id = id_temp;

    return Result::kOk;
}
ST7735::ST7735(SpiMaster &spi, Pin &dcPin)
    : St77xx(spi, dcPin) {

      };
}  // namespace wibot

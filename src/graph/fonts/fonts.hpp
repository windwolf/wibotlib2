#pragma once

#include "graph.hpp"

namespace wibot {

enum FONT_MEMORY_LAYOUT_DIRECTION {
    FONT_MEMORY_LAYOUT_DIRECTION_HORIZONTAL = 0,
    FONT_MEMORY_LAYOUT_DIRECTION_VERTICAL,
};

struct FontInfo {
    const u8                    *table;
    u16                          width;
    u16                          height;
    FONT_MEMORY_LAYOUT_DIRECTION direction;
};

extern FontInfo Font17x24;

extern FontInfo Font14x20;

extern FontInfo Font11x16;

extern FontInfo Font7x12;

extern FontInfo Font5x8;

extern FontInfo Font6x8_v;

extern FontInfo Font8x16_v;

struct SpaceInfo {
    u16 x;
    u16 y;
    u16 width;
    u16 height;
};

struct FontDrawInfo {
    Color foreColor;
    Color backColor;
    u8    spacing;
    u8    lineSpacing;
};

enum class PixelSize : u32 {
    Bit1  = 0x00000001,
    Bit8  = 0x000000FF,
    Bit16 = 0x0000FFFF,
    Bit24 = 0x00FFFFFF,
    Bit32 = 0xFFFFFFFF,
};

enum class CanvasMemoryLayoutDirection {
    Horizontal = 0,
    Vertical,
};

struct CanvasInfo {
    u16                         width;
    u16                         height;
    PixelSize                   pixelSize;
    CanvasMemoryLayoutDirection direction;
};

bool FONTS_CalcSpace(u16 x, u16 y, const char *str, FontInfo *fontInfo, u8 spacing,
                     SpaceInfo *spaceInfo);

bool FONTS_FillData(u8 *buffer, CanvasInfo *canvas, u16 x, u16 y, const char *str,
                    FontInfo *fontInfo, FontDrawInfo *fontDrawInfo);
}  // namespace wibot

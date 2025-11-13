#include "fonts/fonts.hpp"

#include "string.h"

namespace wibot {
static inline void setPixel(u8 *buffer, u32 memOffset, PixelSize pixelSize, Color color) {
    if (pixelSize == PixelSize::Bit8) {
        buffer[memOffset] = (u8)(color.value);
    } else if (pixelSize == PixelSize::Bit16) {
        ((u16 *)buffer)[memOffset] = (u16)(color.value);
    } else if (pixelSize == PixelSize::Bit32) {
        ((u32 *)buffer)[memOffset] = (u32)(color.value);
    } else if (pixelSize == PixelSize::Bit24) {
        buffer[memOffset * 3]     = (u8)(color.value >> 16);
        buffer[memOffset * 3 + 1] = (u8)(color.value >> 8);
        buffer[memOffset * 3 + 2] = (u8)(color.value);
    } else if (pixelSize == PixelSize::Bit1) {
        if (color.value == 1) {
            ((u8 *)buffer)[memOffset / 8] |= (u8)(color.value) << (7 - (memOffset % 8));
        } else {
            ((u8 *)buffer)[memOffset / 8] &= ~((u8)(color.value) << (7 - (memOffset % 8)));
        }
    }
}

/**
 * @brief whole font size in bytes
 *
 * @param fontInfo
 * @return u8
 */
[[maybe_unused]] static inline u8 getFontDataSize(FontInfo *fontInfo) {
    if (fontInfo->direction == FONT_MEMORY_LAYOUT_DIRECTION_HORIZONTAL) {
        return (fontInfo->width + 7) / 8 * fontInfo->height;
    } else {
        return (fontInfo->height + 7) / 8 * fontInfo->width;
    }
}

/**
 * @brief font data size in one row in bytes
 *
 * @param fontInfo
 * @return u8
 */
[[maybe_unused]] static inline u8 getFontRowSize(FontInfo *fontInfo) {
    if (fontInfo->direction == FONT_MEMORY_LAYOUT_DIRECTION_HORIZONTAL) {
        return (fontInfo->width + 7) / 8;
    } else {
        return (fontInfo->height + 7) / 8;
    }
}

bool FONTS_CalcSpace(u16 x, u16 y, const char *str, FontInfo *fontInfo, u8 spacing,
                     SpaceInfo *spaceInfo) {
    u32 len = strlen(str);
    if (len == 0) {
        spaceInfo->x      = x;
        spaceInfo->y      = y;
        spaceInfo->width  = 0;
        spaceInfo->height = 0;
        return true;
    } else {
        spaceInfo->x      = x;
        spaceInfo->y      = y;
        spaceInfo->width  = len * (fontInfo->width + spacing) - 1;
        spaceInfo->height = fontInfo->height;
        return true;
    }
}

bool FONTS_FillData(u8 *buffer, CanvasInfo *canvas, u16 x, u16 y, const char *str,
                    FontInfo *fontInfo, FontDrawInfo *fontDrawInfo) {
    // if (canvas->direction != fontInfo->direction)
    // {
    //     return false;
    // }
    u16 len = strlen(str);
    if (len == 0) {
        return true;
    } else {
        if ((y + fontInfo->height) > canvas->height) {
            return false;
        }
        u8 fontMemSize    = (fontInfo->direction == FONT_MEMORY_LAYOUT_DIRECTION_HORIZONTAL)
                                ? (fontInfo->width + 7) / 8 * fontInfo->height
                                : (fontInfo->height + 7) / 8 * fontInfo->width;
        u8 fontMemRowSize = (fontInfo->direction == FONT_MEMORY_LAYOUT_DIRECTION_HORIZONTAL)
                                ? (fontInfo->width + 7) / 8
                                : (fontInfo->height + 7) / 8;

        u8 fontMemWidth  = (fontInfo->direction == FONT_MEMORY_LAYOUT_DIRECTION_HORIZONTAL)
                               ? fontInfo->width
                               : fontInfo->height;
        u8 fontMemHeight = (fontInfo->direction == FONT_MEMORY_LAYOUT_DIRECTION_HORIZONTAL)
                               ? fontInfo->height
                               : fontInfo->width;

        u8 lineSpacingMemWidth  = (fontInfo->direction == FONT_MEMORY_LAYOUT_DIRECTION_HORIZONTAL)
                                      ? fontDrawInfo->lineSpacing
                                      : fontInfo->height;
        u8 lineSpacingMemHeight = (fontInfo->direction == FONT_MEMORY_LAYOUT_DIRECTION_HORIZONTAL)
                                      ? fontInfo->height
                                      : fontDrawInfo->lineSpacing;

        PixelSize pixelSize = canvas->pixelSize;
        u8        memX      = canvas->direction == CanvasMemoryLayoutDirection::Horizontal ? x : y;
        u8        memY      = canvas->direction == CanvasMemoryLayoutDirection::Horizontal ? y : x;
        u8        canvasMemWidth = canvas->direction == CanvasMemoryLayoutDirection::Horizontal
                                       ? canvas->width
                                       : canvas->height;
        // u8 canvasMemHeight = canvas->direction ==
        // CanvasMemoryLayoutDirection::Horizontal ? canvas->height :
        // canvas->width;
        for (u16 i = 0; i < len; i++) {
            // draw spacing;
            if (i != 0) {
                if ((x + fontDrawInfo->spacing) >= canvas->width) {
                    return false;
                }
                for (u16 j = 0; j < lineSpacingMemHeight; j++) {
                    u32 y_index = memY + j;
                    for (u16 k = 0; k < lineSpacingMemWidth; k++) {
                        u32 x_index   = memX + k;
                        u32 memOffset = y_index * canvasMemWidth + x_index;
                        setPixel(buffer, memOffset, pixelSize, fontDrawInfo->backColor);
                    }
                }
                x += fontDrawInfo->spacing;
            }

            // draw charactor;
            if ((x + fontInfo->width) > canvas->width) {
                return false;
            }
            memX = canvas->direction == CanvasMemoryLayoutDirection::Horizontal ? x : y;
            memY = canvas->direction == CanvasMemoryLayoutDirection::Horizontal ? y : x;

            const u8 *fontData = &(fontInfo->table[(str[i] - 32) * fontMemSize]);

            for (u16 j = 0; j < fontMemHeight; j++) {
                u32 y_index = memY + j;
                for (u16 k = 0; k < fontMemWidth; k++) {
                    u32   x_index = memX + k;
                    u32   offset  = y_index * canvasMemWidth + x_index;
                    Color color;
                    if (fontData[j * fontMemRowSize + k / 8] & (1 << (fontMemWidth - k - 1))) {
                        color = fontDrawInfo->foreColor;
                    } else {
                        color = fontDrawInfo->backColor;
                    }

                    setPixel(buffer, offset, pixelSize, color);
                }
            }
            x += fontInfo->width;
        }
        return true;
    }
}
}  // namespace wibot

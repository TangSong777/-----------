#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

typedef struct
{
    const uint8_t width;
    const uint8_t height;
    const uint8_t *data;
} FontDef;

extern const FontDef Font_Custom;
extern const FontDef Font_6x12;
extern const FontDef Font_8x16;
extern const FontDef Font_CN;

extern const uint8_t BMP_YHL[];
extern const uint8_t BMP_wwfox[];
extern const uint8_t BMP_LongZhongNiao[];
#endif // __FONT_H__
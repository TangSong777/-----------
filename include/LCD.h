#ifndef __LCD_H__
#define __LCD_H__
#include "Arduino.h"
#include "SPI.h"
#include "Font.h"

// 定义LCD相关的引脚
#define LCD_RST_PIN 15
#define LCD_DC_PIN 16
#define LCD_CS_PIN 14
#define LCD_SCLK_PIN 12
#define LCD_MOSI_PIN 13

#define LCD_XSTART 2
#define LCD_YSTART 1
#define LCD_WIDTH 128
#define LCD_HEIGHT 160

// Screen Direction
#define LCD_ROTATION 0
// Color Mode: RGB or BGR
#define LCD_MADCTL_RGB 0x00
#define LCD_MADCTL_BGR 0x08
#define LCD_MADCTL_MODE LCD_MADCTL_RGB
// Color Inverse: 0=NO, 1=YES
#define LCD_INVERSE 0

// Color definitions
#define LCD_BLACK 0x0000
#define LCD_BLUE 0x001F
#define LCD_RED 0xF800
#define LCD_GREEN 0x07E0
#define LCD_CYAN 0x07FF
#define LCD_MAGENTA 0xF81F
#define LCD_YELLOW 0xFFE0
#define LCD_WHITE 0xFFFF
#define LCD_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

void LCD_Init();
void LCD_DrawChar(uint8_t x, uint8_t y, const char c, uint16_t color, uint16_t bgColor, const FontDef *font);
void LCD_DrawString(uint8_t x, uint8_t y, const char *str, uint16_t color, uint16_t bgColor, const FontDef *font);
void LCD_DrawCNChar(uint8_t x, uint8_t y, const uint8_t index, uint16_t color, uint16_t bgColor);
void LCD_DrawCNString(uint8_t x, uint8_t y, const uint8_t indexs[], uint8_t showLength, uint8_t maxLength, uint16_t color, uint16_t bgColor);
void LCD_DrawNumbers(uint8_t x, uint8_t y, int num, int maxlen, uint16_t color, uint16_t bgColor, const FontDef *font);
void LCD_DrawPaddedNumbers(uint8_t x, uint8_t y, int num, int maxlen, uint16_t color, uint16_t bgColor, const FontDef *font);
void LCD_DrawImage(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *image);
void LCD_SetPixel(uint8_t x, uint8_t y, uint16_t color);
void LCD_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color);
void LCD_DrawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color, uint16_t bgColor);
void LCD_DrawRectangleBorder(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color);
void LCD_DrawFilledRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color);
float LCD_DrawProgressBar(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t borderColor, uint16_t unfillColor, uint16_t fillColor, int num, int maxnum);
void LCD_FillScreen(uint16_t color);
#endif // __LCD_H__
#include "LCD.h"
#include <SPI.h>

#define LCD_SLPOUT 0x11
#define LCD_FRMCTR1 0xB1
#define LCD_FRMCTR2 0xB2
#define LCD_FRMCTR3 0xB3
#define LCD_INVCTR 0xB4
#define LCD_PWCTR1 0xC0
#define LCD_PWCTR2 0xC1
#define LCD_PWCTR3 0xC2
#define LCD_PWCTR4 0xC3
#define LCD_PWCTR5 0xC4
#define LCD_VMCTR1 0xC5
#define LCD_COLMOD 0x3A
#define LCD_GMCTRP1 0xE0
#define LCD_GMCTRN1 0xE1
#define LCD_NORON 0x13
#define LCD_DISPON 0x29
#define LCD_CASET 0x2A
#define LCD_RASET 0x2B
#define LCD_RAMWR 0x2C
#define LCD_INVOFF 0x20
#define LCD_INVON 0x21

#define MADCTL_MY 0x80 // 旋转 180 度
#define MADCTL_MX 0x40 // 旋转 90 度
#define MADCTL_MV 0x10 // 旋转 270 度
#define MADCTL_ML 0x08 // 旋转 0 度

// ========================== 底层通信函数 ==========================
void LCD_Reset()
{
    digitalWrite(LCD_RST_PIN, LOW);
    delay(100); // 等待100ms
    digitalWrite(LCD_RST_PIN, HIGH);
    delay(100); // 等待100ms
}
void LCD_WriteCommand(uint8_t cmd)
{
    digitalWrite(LCD_DC_PIN, LOW);  // 选择命令模式
    digitalWrite(LCD_CS_PIN, LOW);  // 选中从设备
    SPI.transfer(cmd);              // 发送命令
    digitalWrite(LCD_CS_PIN, HIGH); // 取消选中从设备
}
void LCD_WriteByte8(uint8_t data)
{
    digitalWrite(LCD_DC_PIN, HIGH); // 选择数据模式
    digitalWrite(LCD_CS_PIN, LOW);  // 选中从设备
    SPI.transfer(data);             // 发送数据
    digitalWrite(LCD_CS_PIN, HIGH); // 取消选中从设备
}
void LCD_WriteByte16(uint16_t data)
{
    digitalWrite(LCD_DC_PIN, HIGH); // 选择数据模式
    digitalWrite(LCD_CS_PIN, LOW);  // 选中从设备
    SPI.transfer16(data);           // 发送数据
    digitalWrite(LCD_CS_PIN, HIGH); // 取消选中从设备
}
void LCD_WriteData8(const uint8_t *data, size_t data_size)
{
    digitalWrite(LCD_DC_PIN, HIGH); // 选择数据模式
    digitalWrite(LCD_CS_PIN, LOW);  // 选中从设备
    for (size_t i = 0; i < data_size; ++i)
    {
        SPI.transfer(data[i]); // 发送数据
    }
    digitalWrite(LCD_CS_PIN, HIGH); // 取消选中从设备
}
void LCD_WriteData16(const uint16_t *data, size_t data_size)
{
    digitalWrite(LCD_DC_PIN, HIGH); // 选择数据模式
    digitalWrite(LCD_CS_PIN, LOW);  // 选中从设备
    for (size_t i = 0; i < data_size; ++i)
    {
        SPI.transfer16(data[i]); // 发送数据
    }
    digitalWrite(LCD_CS_PIN, HIGH); // 取消选中从设备
}
void LCD_SetRotation(uint8_t rotation)
{
    uint8_t madctl = 0;
    switch (rotation)
    {
    case 0: // 0 度
        madctl = MADCTL_ML;
        break;
    case 1: // 90 度
        madctl = MADCTL_MX | MADCTL_MY;
        break;
    case 2: // 180 度
        madctl = MADCTL_ML | MADCTL_MY;
        break;
    case 3: // 270 度
        madctl = MADCTL_MX | MADCTL_ML;
        break;
    default:
        madctl = MADCTL_ML; // 默认设置为 0 度
        break;
    }
    LCD_WriteCommand(0x36); // MADCTL 命令
    LCD_WriteByte8(madctl); // 发送 MADCTL 值
}
// ========================== LCD驱动函数 ==========================
void LCD_Init()
{
    // 初始化SPI引脚
    pinMode(LCD_RST_PIN, OUTPUT);
    pinMode(LCD_DC_PIN, OUTPUT);
    pinMode(LCD_CS_PIN, OUTPUT);

    // 设置初始状态
    digitalWrite(LCD_RST_PIN, HIGH);
    digitalWrite(LCD_DC_PIN, HIGH);
    digitalWrite(LCD_CS_PIN, HIGH);

    // 开始SPI通信
    SPI.begin(LCD_SCLK_PIN, 19, LCD_MOSI_PIN, LCD_CS_PIN);
    // 设置SPI速度、数据模式等（如果需要）
    SPI.setClockDivider(SPI_CLOCK_DIV4); // 设置时钟分频器
    SPI.setDataMode(SPI_MODE0);          // 设置数据模式

    LCD_Reset();

    LCD_WriteCommand(LCD_SLPOUT);
    delay(120);

    LCD_WriteCommand(LCD_FRMCTR1);
    LCD_WriteByte8(0x01);
    LCD_WriteByte8(0x2C);
    LCD_WriteByte8(0x2D);

    LCD_WriteCommand(LCD_FRMCTR2);
    LCD_WriteByte8(0x01);
    LCD_WriteByte8(0x2C);
    LCD_WriteByte8(0x2D);

    LCD_WriteCommand(LCD_FRMCTR3);
    LCD_WriteByte8(0x01);
    LCD_WriteByte8(0x2C);
    LCD_WriteByte8(0x2D);
    LCD_WriteByte8(0x01);
    LCD_WriteByte8(0x2C);
    LCD_WriteByte8(0x2D);

    LCD_WriteCommand(LCD_INVCTR);
    LCD_WriteByte8(0x07);

    LCD_WriteCommand(LCD_PWCTR1);
    LCD_WriteByte8(0xA2);
    LCD_WriteByte8(0x02);
    LCD_WriteByte8(0x84);

    LCD_WriteCommand(LCD_PWCTR2);
    LCD_WriteByte8(0xC5);

    LCD_WriteCommand(LCD_PWCTR3);
    LCD_WriteByte8(0x0A);
    LCD_WriteByte8(0x00);

    LCD_WriteCommand(LCD_PWCTR4);
    LCD_WriteByte8(0x8A);
    LCD_WriteByte8(0x2A);

    LCD_WriteCommand(LCD_PWCTR5);
    LCD_WriteByte8(0x8A);
    LCD_WriteByte8(0xEE);

    LCD_WriteCommand(LCD_VMCTR1);
    LCD_WriteByte8(0x0E);

    LCD_WriteCommand(LCD_INVERSE ? LCD_INVON : LCD_INVOFF);

    LCD_WriteCommand(LCD_COLMOD);
    LCD_WriteByte8(0x05);

    LCD_WriteCommand(LCD_CASET);
    LCD_WriteByte8(0x00);
    LCD_WriteByte8(0x00);
    LCD_WriteByte8(0x00);
    LCD_WriteByte8(0x7F);

    LCD_WriteCommand(LCD_RASET);
    LCD_WriteByte8(0x00);
    LCD_WriteByte8(0x00);
    LCD_WriteByte8(0x00);
    LCD_WriteByte8(0x9F);

    LCD_WriteCommand(LCD_GMCTRP1);
    LCD_WriteByte8(0x02);
    LCD_WriteByte8(0x1C);
    LCD_WriteByte8(0x07);
    LCD_WriteByte8(0x12);
    LCD_WriteByte8(0x37);
    LCD_WriteByte8(0x32);
    LCD_WriteByte8(0x29);
    LCD_WriteByte8(0x2D);
    LCD_WriteByte8(0x29);
    LCD_WriteByte8(0x25);
    LCD_WriteByte8(0x2B);
    LCD_WriteByte8(0x39);
    LCD_WriteByte8(0x00);
    LCD_WriteByte8(0x01);
    LCD_WriteByte8(0x03);
    LCD_WriteByte8(0x10);

    LCD_WriteCommand(LCD_GMCTRN1);
    LCD_WriteByte8(0x03);
    LCD_WriteByte8(0x1D);
    LCD_WriteByte8(0x07);
    LCD_WriteByte8(0x06);
    LCD_WriteByte8(0x2E);
    LCD_WriteByte8(0x2C);
    LCD_WriteByte8(0x29);
    LCD_WriteByte8(0x2D);
    LCD_WriteByte8(0x2E);
    LCD_WriteByte8(0x2E);
    LCD_WriteByte8(0x37);
    LCD_WriteByte8(0x3F);
    LCD_WriteByte8(0x00);
    LCD_WriteByte8(0x00);
    LCD_WriteByte8(0x02);
    LCD_WriteByte8(0x10);

    LCD_WriteCommand(LCD_NORON);
    delay(10);

    LCD_WriteCommand(LCD_DISPON);
    delay(10);

    LCD_FillScreen(LCD_BLACK);
}
void LCD_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t width, uint8_t height)
{
    x0 += LCD_XSTART;
    y0 += LCD_YSTART;

    uint8_t x1 = x0 + width - 1;
    uint8_t y1 = y0 + height - 1;

    LCD_WriteCommand(LCD_CASET);
    uint8_t data[] = {0x00, x0, 0x00, x1};
    LCD_WriteData8(data, sizeof(data));

    LCD_WriteCommand(LCD_RASET);
    data[1] = y0;
    data[3] = y1;
    LCD_WriteData8(data, sizeof(data));
}
// ========================== 文字绘制函数 ==========================
void LCD_DrawChar(uint8_t x, uint8_t y, const char c, uint16_t color, uint16_t bgColor, const FontDef *font)
{
    uint32_t b;
    LCD_SetAddressWindow(x, y, font->width, font->height);
    LCD_WriteCommand(LCD_RAMWR);

    for (uint8_t i = 0; i < font->height; i++)
    {
        b = font->data[(c - 32) * font->height + i];
        for (int j = 7; j >= 8 - font->width; j--)
        {
            if ((b >> j) & 0x01)
                LCD_WriteByte16(color);
            else
                LCD_WriteByte16(bgColor);
        }
    }
}
void LCD_DrawString(uint8_t x, uint8_t y, const char *str, uint16_t color, uint16_t bgColor, const FontDef *font)
{
    while (*str)
    {
        if (x + font->width > LCD_WIDTH)
        {
            x = 0;
            y += font->height;
        }

        if (y + font->height > LCD_HEIGHT)
        {
            break;
        }

        LCD_DrawChar(x, y, *str, color, bgColor, font);
        x += font->width;
        str++;
    }
}
void LCD_DrawCNChar(uint8_t x, uint8_t y, const uint8_t index, uint16_t color, uint16_t bgColor)
{
    uint32_t b;
    LCD_SetAddressWindow(x, y, 16, 16);
    LCD_WriteCommand(LCD_RAMWR);

    for (uint8_t i = 0; i < 32; i++)
    {
        b = (&Font_CN)->data[index * 32 + i];
        for (int j = 7; j >= 0; j--)
        {
            if ((b >> j) & 0x01)
                LCD_WriteByte16(color);
            else
                LCD_WriteByte16(bgColor);
        }
    }
}
void LCD_DrawCNString(uint8_t x, uint8_t y, const uint8_t indexs[], uint8_t showLength, uint8_t maxLength, uint16_t color, uint16_t bgColor)
{
    for (uint8_t i = 0; i < showLength; i++)
        LCD_DrawCNChar(x + (i * 16), y, indexs[i], color, bgColor);
    LCD_DrawFilledRectangle(x + showLength * 16, y, (maxLength - showLength) * 16, 16, bgColor);
}
void LCD_DrawNumbers(uint8_t x, uint8_t y, int num, int maxlen, uint16_t color, uint16_t bgColor, const FontDef *font)
{
    char str[maxlen + 1];                           // +1 for the null terminator
    int len = snprintf(str, maxlen + 1, "%d", num); // Use maxlen + 1 to ensure proper termination

    if (len < 0 || len > maxlen) // 如果len>maxlen，那么只显示num从左往右的maxlen位
    {
        len = maxlen;
        str[len] = '\0';
    }
    LCD_DrawFilledRectangle(x + font->width * len, y, font->width * (maxlen - len), font->height, bgColor);
    LCD_DrawString(x, y, str, color, bgColor, font);
}
void LCD_DrawPaddedNumbers(uint8_t x, uint8_t y, int num, int maxlen, uint16_t color, uint16_t bgColor, const FontDef *font)
{
    char str[maxlen + 1];
    int len = snprintf(str, maxlen + 1, "%0*d", maxlen, num);

    if (len < 0 || len > maxlen)
    {
        len = maxlen;
        str[len] = '\0';
    }

    int rectWidth = font->width * (maxlen - len);
    if (rectWidth < 0)
        rectWidth = 0;
    LCD_DrawFilledRectangle(x, y, rectWidth, font->height, bgColor);
    LCD_DrawString(x + rectWidth, y, str, color, bgColor, font);
}
void LCD_DrawImage(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *image)
{
    LCD_SetAddressWindow(x, y, width, height);

    LCD_WriteCommand(LCD_RAMWR);

    LCD_WriteData8((uint8_t *)image, sizeof(uint16_t) * width * height);
}
// ========================== 图形绘制函数 ==========================
void LCD_SetPixel(uint8_t x, uint8_t y, uint16_t color)
{
    LCD_SetAddressWindow(x, y, 1, 1);
    LCD_WriteCommand(LCD_RAMWR);
    LCD_WriteByte16(color);
}
void LCD_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color)
{
    if (x1 > LCD_WIDTH || y1 > LCD_HEIGHT || x2 > LCD_WIDTH || y2 > LCD_HEIGHT)
        return;
    static uint8_t temp = 0;
    if (x1 == x2)
    {
        // if (y1 > y2)
        //     y1 ^= y2 ^= y1 ^= y2;
        // for (uint8_t y = y1; y <= y2; y++)
        //     LCD_SetPixel(x1, y, color);

        static uint8_t buff[2];
        buff[0] = color >> 8;
        buff[1] = color & 0xFF;

        LCD_SetAddressWindow(x1, y1, 1, y2 - y1 + 1);
        LCD_WriteCommand(LCD_RAMWR);

        for (int i = 0; i < y2 - y1 + 1; i++)
            LCD_WriteData8(buff, sizeof(buff));
    }
    else if (y1 == y2)
    {
        // if (x1 > x2)
        //     x1 ^= x2 ^= x1 ^= x2;
        // for (uint8_t x = x1; x <= x2; x++)
        //     LCD_SetPixel(x, y1, color);

        static uint8_t buff[2];
        buff[0] = color >> 8;
        buff[1] = color & 0xFF;

        LCD_SetAddressWindow(x1, y1, x2 - x1 + 1, 1);
        LCD_WriteCommand(LCD_RAMWR);

        for (int i = 0; i < x2 - x1 + 1; i++)
            LCD_WriteData8(buff, sizeof(buff));
    }
    else
    {
        // Bresenham直线算法
        int16_t dx = x2 - x1;
        int16_t dy = y2 - y1;
        int16_t ux = ((dx > 0) << 1) - 1;
        int16_t uy = ((dy > 0) << 1) - 1;
        int16_t x = x1, y = y1, eps = 0;
        dx = abs(dx);
        dy = abs(dy);
        if (dx > dy)
        {
            for (x = x1; x != x2; x += ux)
            {
                LCD_SetPixel(x, y, color);
                eps += dy;
                if ((eps << 1) >= dx)
                {
                    y += uy;
                    eps -= dx;
                }
            }
        }
        else
        {
            for (y = y1; y != y2; y += uy)
            {
                LCD_SetPixel(x, y, color);
                eps += dx;
                if ((eps << 1) >= dy)
                {
                    x += ux;
                    eps -= dy;
                }
            }
        }
    }
}
void LCD_DrawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color, uint16_t bgColor)
{
    if (x > LCD_WIDTH || y > LCD_HEIGHT)
        return;
    LCD_DrawLine(x, y, x + width - 1, y, color);
    LCD_DrawLine(x, y + height - 1, x + width - 1, y + height - 1, color);
    LCD_DrawLine(x, y, x, y + height - 1, color);
    LCD_DrawLine(x + width - 1, y, x + width - 1, y + height - 1, color);
    LCD_DrawFilledRectangle(x + 1, y + 1, width - 2, height - 2, bgColor);
}
void LCD_DrawRectangleBorder(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color)
{
    if (x > LCD_WIDTH || y > LCD_HEIGHT)
        return;
    LCD_DrawLine(x, y, x + width - 1, y, color);
    LCD_DrawLine(x, y + height - 1, x + width - 1, y + height - 1, color);
    LCD_DrawLine(x, y, x, y + height - 1, color);
    LCD_DrawLine(x + width - 1, y, x + width - 1, y + height - 1, color);
}
void LCD_DrawFilledRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color)
{
    if (x > LCD_WIDTH || y > LCD_HEIGHT)
        return;
    static uint8_t buff[LCD_WIDTH * 2];
    uint16_t i = 0;

    for (i = 0; i < width; i++)
    {
        buff[i * 2] = color >> 8;
        buff[i * 2 + 1] = color & 0xFF;
    }
    if (x + width > LCD_WIDTH)
        width = LCD_WIDTH - x;
    if (y + height > LCD_HEIGHT)
        height = LCD_HEIGHT - y;

    LCD_SetAddressWindow(x, y, width, height);
    LCD_WriteCommand(LCD_RAMWR);
    // Write the color data
    for (i = 0; i < height; i++)
        LCD_WriteData8(buff, sizeof(uint16_t) * width);
}
float LCD_DrawProgressBar(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t borderColor, uint16_t unfillColor, uint16_t fillColor, int num, int maxnum)
{
    float progress = num * 1.0f / maxnum;
    uint8_t fillArea_width = progress * (width - 2), unfillArea_width = width - fillArea_width - 2;
    LCD_DrawRectangleBorder(x, y, width, height, borderColor);
    LCD_DrawFilledRectangle(x + 1, y + 1, fillArea_width, height - 2, fillColor);
    LCD_DrawFilledRectangle(x + fillArea_width + 1, y + 1, unfillArea_width, height - 2, unfillColor);
    return progress;
}
void LCD_FillScreen(uint16_t color)
{
    LCD_DrawFilledRectangle(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}
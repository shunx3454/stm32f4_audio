//
// Created by Zhou 2024/1/7
//

#include "lcd.h"
#include <stdio.h>
#include "periph.h"
#include "FreeRTOS.h"
#include "task.h"

// 字体
extern const unsigned char ASCII1206[95][12];
extern const unsigned char ASCII1608[95][16];
extern const unsigned char ASCII2412[95][48];
extern const unsigned char ASCII3216[95][64];

__IO uint16_t BACKCOLOR = BLACK;
__IO uint16_t POINT = WHITE;

void lcd_bl_pin(void)
{
    /* LCD_BL PIN */
    LCD_BLK_GPIO_CLK_ENABLE();
    GPIO_InitTypeDef gpio;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pin = GPIO_PIN_15;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_BLK_GPIO_Port, &gpio);
}

/* LCD Write byte DATA */
__attribute__((optimize("O0"))) void LCD_WR_DATAS(uint8_t *pBuff, uint16_t size)
{
    uint16_t i = 0;
    for (i = 0; i < size; i++)
    {
        LCD_WR_DATA(*(__IO uint16_t *)pBuff);
        pBuff += 2;
    }
}

/* LCD Write byte DATA */
__attribute__((optimize("O0"))) void LCD_WR_DATA(__IO uint16_t dat)
{
    dat = dat;
    *(__IO uint16_t *)FSMC_LCD_RAM = dat;
}

/* LCD Write CMD byte */
__attribute__((optimize("O0"))) void LCD_WR_REG(__IO uint16_t cmd)
{
    cmd = cmd;
    *(__IO uint16_t *)FSMC_LCD_REG = cmd;
}

/* LCD Read CMD byte */
__attribute__((optimize("O0"))) uint16_t LCD_RD_DATA(void)
{
    return *(__IO uint16_t *)FSMC_LCD_RAM;
}

/* LCD Init */
void lcd_init(void) ////ST7789V2
{
    lcd_bl_pin();

    LCD_BLK_LOW;
    LCD_WR_REG(0x11); // Sleep Out
    lcd_delay(120);   // DELAY120ms

    LCD_WR_REG(0x36); // set display direction
#if (LCD_DIRECTION == LCD_DIRECT_0DEG)
    LCD_WR_DATA(0x00);
#elif (LCD_DIRECTION == LCD_DIRECT_90DEG)
    LCD_WR_DATA(0x70);
#elif (LCD_DIRECTION == LCD_DIRECT_180DEG)
    LCD_WR_DATA(0xC0);
#else
    LCD_WR_DATA(0xa0);
#endif

    /** -----------------------ST7789V Frame rate setting----------------- **/
    LCD_WR_REG(0x3A);
    LCD_WR_DATA(0x05);

    LCD_WR_REG(0xB2);
    LCD_WR_DATA(0x0C);
    LCD_WR_DATA(0x0C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x33);
    LCD_WR_DATA(0x33);

    LCD_WR_REG(0xB7);
    LCD_WR_DATA(0x71);

    LCD_WR_REG(0xBB);
    LCD_WR_DATA(0x3B); // Vcom=1.35V

    LCD_WR_REG(0xC0);
    LCD_WR_DATA(0x2C);

    LCD_WR_REG(0xC2);
    LCD_WR_DATA(0x01);

    LCD_WR_REG(0xC3);
    LCD_WR_DATA(0x13); // GVDD=4.8V  颜色深度

    LCD_WR_REG(0xC4);
    LCD_WR_DATA(0x20); // VDV, 0x20:0v

    LCD_WR_REG(0xC6);
    LCD_WR_DATA(0x0F); // 0x0F:60Hz

    LCD_WR_REG(0xD0);
    LCD_WR_DATA(0xA4);
    LCD_WR_DATA(0xA1);

    LCD_WR_REG(0xE0);
    LCD_WR_DATA(0xD0);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x0A);
    LCD_WR_DATA(0x0D);
    LCD_WR_DATA(0x0B);
    LCD_WR_DATA(0x07);
    LCD_WR_DATA(0x21);
    LCD_WR_DATA(0x33);
    LCD_WR_DATA(0x39);
    LCD_WR_DATA(0x39);
    LCD_WR_DATA(0x16);
    LCD_WR_DATA(0x16);
    LCD_WR_DATA(0x1F);
    LCD_WR_DATA(0x3C);

    LCD_WR_REG(0xE1);
    LCD_WR_DATA(0xD0);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x21);
    LCD_WR_DATA(0x32);
    LCD_WR_DATA(0x38);
    LCD_WR_DATA(0x16);
    LCD_WR_DATA(0x14);
    LCD_WR_DATA(0x14);
    LCD_WR_DATA(0x20);
    LCD_WR_DATA(0x3D);

    LCD_WR_REG(0x21);

    /** end */
    lcd_clear(BLACK);

    LCD_BLK_HIGH;
    LCD_WR_REG(0x29); // 开启显示
}

void lcd_clear(uint16_t color)
{
    lcd_set_window(0, LCD_CURRENT_COLUMNS - 1, 0, LCD_CURRENT_LINES - 1);
    lcd_wr_gram();
    for (uint32_t i = 0; i < LCD_CURRENT_LINES * LCD_CURRENT_COLUMNS; i++)
        LCD_WR_DATA(color);
}

void lcd_fill(uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey, uint16_t color)
{
    uint16_t width = ex - sx + 1;
    uint16_t height = ey - sy + 1;
    (height >= LCD_CURRENT_LINES) ? (height = LCD_CURRENT_LINES) : (height = height);
    (width >= LCD_CURRENT_COLUMNS) ? (width = LCD_CURRENT_COLUMNS) : (width = width);

    /* set area window */
    lcd_set_window(sx, ex, sy, ey);
    lcd_wr_gram();
    for (uint32_t i = 0; i < height * width; i++)
    {
        LCD_WR_DATA(color);
    }
}

void lcd_set_point(uint16_t x, uint16_t y)
{
    lcd_set_window(x, x, y, y);
}

void lcd_set_window(uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey)
{
    LCD_WR_REG(0x2A);
    LCD_WR_DATA(sx >> 8); // start Xpos
    LCD_WR_DATA(sx & 0xff);

    LCD_WR_DATA(ex >> 8); // end Xpos
    LCD_WR_DATA(ex & 0xff);

    LCD_WR_REG(0x2B);
    LCD_WR_DATA(sy >> 8); // start Ypos
    LCD_WR_DATA(sy & 0xff);

    LCD_WR_DATA(ey >> 8); // end Ypos
    LCD_WR_DATA(ey & 0xff);
}

void lcd_wr_gram(void)
{
    LCD_WR_REG(0x2c);
}

void lcd_fast_point(uint16_t x, uint16_t y, uint16_t color)
{
    lcd_set_point(x, y);
    lcd_wr_gram();
    LCD_WR_DATA(color);
}

void lcd_showchar(uint16_t x, uint16_t y, uint8_t size, uint8_t mode, unsigned char ch)
{
    const unsigned char *pdata;
    uint8_t i, j;
    uint16_t x0 = x;
    uint8_t Bsize = (size == 12 || size == 16) ? size : size << 1;
    ch -= ' ';
    if (size == 12)
        pdata = &ASCII1206[ch][0];
    else if (size == 16)
        pdata = &ASCII1608[ch][0];
    else if (size == 24)
        pdata = &ASCII2412[ch][0];
    else if (size == 32)
        pdata = &ASCII3216[ch][0];
    else
        return;
    for (i = 0; i < Bsize; i++)
    {
        for (j = 0; j < 8; j++)
        {
            if (pdata[i] & (0x01 << j))
                lcd_fast_point(x, y, POINT);
            else if (mode == 0)
                lcd_fast_point(x, y, BACKCOLOR);
            x++;
            if ((x - x0) == size / 2)
            {
                y++;
                x = x0;
                break;
            }
        }
    }
}

void lcd_showstr(uint16_t x, uint16_t y, uint8_t size, uint8_t mode, unsigned char *str)
{
    uint16_t x0;
    x0 = x;
    while (*str != '\0')
    {
        lcd_showchar(x, y, size, mode, *str++);
        x += size / 2;
        if (x >= LCD_CURRENT_COLUMNS)
        {
            x = x0;
            y += size;
            if (y >= LCD_CURRENT_LINES)
                return;
        }
    }
}

void lcd_fill_color(uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey, uint16_t *color)
{
    /* set area window */
    lcd_set_window(sx, ex, sy, ey);
    lcd_wr_gram();
#if !LCD_USE_DMA
    for (uint16_t i = 0; i < (ex - sx + 1) * (ey - sy + 1); i++)
        LCD_WR_DATA(color[i]);
#else
    if ((uint32_t)color % 2)
    {
        SYS_LOG("LCD", "lcd_fill_color *color NO align");
        for (uint16_t i = 0; i < (ex - sx + 1) * (ey - sy + 1); i++)
            LCD_WR_DATA(color[i]);
    }
    else
    {
        HAL_CHECK(HAL_DMA_Start_IT(&g_dma_mem2mem, (uint32_t)color, (uint32_t)FSMC_LCD_RAM, (ex - sx + 1) * (ey - sy + 1)));
        while (g_dma_mem2mem.State != HAL_DMA_STATE_READY && g_dma_mem2mem.ErrorCode == 0)
        {
        }
    }
#endif
}

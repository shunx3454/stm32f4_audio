//
// Created by Zhou 2024/1/7
//

#ifndef __LCD_FSMC_IO_H
#define __LCD_FSMC_IO_H

#include <stdint.h>
#include "main.h"
#include "bord_config.h"

#define lcd_delay(x) HAL_Delay(x)

#define FSMC_LCD_REG (uint32_t)0x6C000000
#define FSMC_LCD_RAM (uint32_t)0x6C000080

#define LCD_BLK_LOW HAL_GPIO_WritePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin, GPIO_PIN_RESET)
#define LCD_BLK_HIGH HAL_GPIO_WritePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin, GPIO_PIN_SET)

void LCD_WR_DATA(__IO uint16_t dat);
void LCD_WR_REG(__IO uint16_t cmd);
uint16_t LCD_RD_DATA(void);
void LCD_WR_DATAS(uint8_t *pBuff, uint16_t size);

extern __IO uint16_t BACKCOLOR;
extern __IO uint16_t POINT;

#define DMATRANS_MAX_NUM 		0x6000
#define TFTLCD_HOR_RES  		(uint16_t)240
#define TFTLCD_VER_RES  		(uint16_t)320
#define TFTLCD_HOR_OFFSET  	0
#define TFTLCD_VER_OFFSET  	0
/**** HOR ****
 *
 *
 *
 * VER
 *
 *
 * ***********/

/* TFT 1.69LCD display direction configuration */
#define LCD_DIRECT_0DEG     0
#define LCD_DIRECT_90DEG    1
#define LCD_DIRECT_180DEG   2
#define LCD_DIRECT_270DEG   3
#define LCD_DIRECTION LCD_SCREEN_ROT

#if(LCD_DIRECTION == LCD_DIRECT_0DEG || LCD_DIRECTION == LCD_DIRECT_180DEG)
#define TFTLCD_LINE_OFFSET 		TFTLCD_VER_OFFSET
#define TFTLCD_COLUMN_OFFSET 	TFTLCD_HOR_OFFSET
#define LCD_CURRENT_LINES 		TFTLCD_VER_RES
#define LCD_CURRENT_COLUMNS 	TFTLCD_HOR_RES
#else
#define TFTLCD_LINE_OFFSET 		TFTLCD_HOR_OFFSET
#define TFTLCD_COLUMN_OFFSET 	TFTLCD_VER_OFFSET
#define LCD_CURRENT_LINES 		TFTLCD_HOR_RES
#define LCD_CURRENT_COLUMNS 	TFTLCD_VER_RES
#endif

// 画笔颜色
#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40 // 棕色
#define BRRED 0XFC07 // 棕红色
#define GRAY 0X8430  // 灰色

// GUI颜色
#define DARKBLUE 0X01CF  // 深蓝色
#define LIGHTBLUE 0X7D7C // 浅蓝色
#define GRAYBLUE 0X5458  // 灰蓝色

// 以上三色为PANEL的颜色
#define LIGHTGREEN 0X841F // 浅绿色
#define LIGHTGRAY 0XEF5B  // 浅灰色(PANNEL)
#define LGRAY 0XC618      // 浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE 0XA651  // 浅灰蓝色(中间层颜色)
#define LBBLUE 0X2B12     // 浅棕蓝色(选择条目的反色)


void lcd_init(void);
void lcd_clear(uint16_t color);
void lcd_fill(uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey, uint16_t color);
void lcd_set_point(uint16_t x, uint16_t y);
void lcd_showchar(uint16_t x, uint16_t y, uint8_t size, uint8_t mode, unsigned char ch);
void lcd_showstr(uint16_t x, uint16_t y, uint8_t size, uint8_t mode, unsigned char *str);
void lcd_fill_color(uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey, uint16_t *color);
void lcd_fast_point(uint16_t x, uint16_t y, uint16_t color);
void lcd_set_window(uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey);
void lcd_wr_gram(void);

#endif //__LCD_FSMC_IO_H
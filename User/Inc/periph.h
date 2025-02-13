//
// Created by 61049 on 2024/9/28.
//

#ifndef DEMO_PERIPH_H
#define DEMO_PERIPH_H

#include "main.h"

/* Export varibales -------------------------------------- */
extern UART_HandleTypeDef g_uart1;
extern SD_HandleTypeDef g_sdio1;
extern TIM_HandleTypeDef g_tim11;
extern I2C_HandleTypeDef g_i2c1;
extern I2S_HandleTypeDef g_i2s2;
extern DMA_HandleTypeDef g_dma_spi2;
extern DMA_HandleTypeDef g_dma_sdio_tx;
extern DMA_HandleTypeDef g_dma_sdio_rx;
extern DMA_HandleTypeDef g_dma_mem2mem;
extern RTC_HandleTypeDef g_rtc;

/* Export function -------------------------------------- */
void gpio_init(void);
void tim_init(void);
void uart_init(void);
int sdio_init(void);
int i2s_init(uint32_t bitswidth, uint32_t audioFreq);
void i2c_init(void);
void sram_init(void);
void dma_sdio_init(void);
void mem_dma_init(void);
void rtc_init(void);

/* Import function */
extern void BSP_SD_TxCpltCallback(SD_HandleTypeDef *hsd);
extern void BSP_SD_RxCpltCallback(SD_HandleTypeDef *hsd);
extern void BSP_SD_ErrorCallback(SD_HandleTypeDef *hsd);
extern void BSP_TIM11_ElapsCallback(TIM_HandleTypeDef* htim);

#endif //DEMO_PERIPH_H

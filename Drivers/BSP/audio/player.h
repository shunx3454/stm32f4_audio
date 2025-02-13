#ifndef __BSP_PLAYER_H_
#define __BSP_PLAYER_H_

#include "main.h"

void player_init(void* QueuePlayerRecv, void (*player_polling_cb)(void));

void player_config(uint8_t bit_per_sample, uint32_t sample_rate);

void I2S_DMA_M0TransferCpltCallback(DMA_HandleTypeDef *hdma);
void I2S_DMA_M1TransferCpltCallback(DMA_HandleTypeDef *hdma);
void I2S_DMA_TransferErrorCallback(DMA_HandleTypeDef *hdma);

void player_stop(void);

void player_begin(void);

void player_set_SPK_val(uint8_t val);

void player_set_HP_val(uint8_t Lval, uint8_t Rval);

#endif
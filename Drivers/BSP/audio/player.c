#include "player.h"
#include "periph.h"
#include "bord_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "wm8978/wm8978.h"

static uint8_t buf0[I2S_DMA_QUEUE_SIZE] __ATTR_EXSRAM __ALIGNED(4);
static uint8_t buf1[I2S_DMA_QUEUE_SIZE] __ATTR_EXSRAM __ALIGNED(4);
static QueueHandle_t QueuePlayerReceive;
static void (*player_polling_callback)(void);

void player_init(void *QueuePlayerRecv, void (*player_polling_cb)(void))
{
    QueuePlayerReceive = *(QueueHandle_t *)QueuePlayerRecv;
    player_polling_callback = player_polling_cb;
}

void player_config(uint8_t bit_per_sample, uint32_t sample_rate)
{
    /* initial hardware */
    uint16_t _DMA_TransCNT = 0;
    if (bit_per_sample == 16)
        _DMA_TransCNT = I2S_DMA_QUEUE_SIZE / 2;
    else if (bit_per_sample == 24 || bit_per_sample == 32)
        _DMA_TransCNT = I2S_DMA_QUEUE_SIZE / 4;
    else
        _DMA_TransCNT = 0;

    i2s_init(bit_per_sample, sample_rate);

    /* start dma-i2s trans buf0 then buf1*/
    HAL_CHECK(HAL_I2S_Transmit_DMA(&g_i2s2, (uint16_t *)buf0, 1));
    /* 取消 I2s DMA请求 */
    HAL_CHECK(HAL_I2S_DMAPause(&g_i2s2));
    /* 设置 HAL_DMA STATUS 暂停 状态，并失能 DMA，中断  */
    /* 不能用HAL_I2S_DMAStop */
    HAL_CHECK(HAL_DMA_Abort_IT(g_i2s2.hdmatx));
    /* 等待中断完成 HAL_DMA STAT ABROT => READY (中断里完成)，DMA 自动关闭所有 中断 */
    while (g_i2s2.hdmatx->State != HAL_DMA_STATE_READY)
    {
    }
    /* 添加DMA中断回调函数 */
    HAL_DMA_RegisterCallback(g_i2s2.hdmatx, HAL_DMA_XFER_CPLT_CB_ID, I2S_DMA_M0TransferCpltCallback);
    HAL_DMA_RegisterCallback(g_i2s2.hdmatx, HAL_DMA_XFER_M1CPLT_CB_ID, I2S_DMA_M1TransferCpltCallback);
    HAL_DMA_RegisterCallback(g_i2s2.hdmatx, HAL_DMA_XFER_ERROR_CB_ID, I2S_DMA_TransferErrorCallback);

    /* 开启DMA双缓冲 */
    HAL_DMAEx_MultiBufferStart_IT(g_i2s2.hdmatx, (uint32_t)buf0, (uint32_t)(&g_i2s2.Instance->DR), (uint32_t)buf1, _DMA_TransCNT);

    /* 重新开启 I2S DMA请求 */
    // HAL_I2S_DMAResume(&g_i2s2);
}

void player_stop(void)
{
    HAL_CHECK(HAL_I2S_DMAPause(&g_i2s2));
}

void player_begin(void)
{
    HAL_CHECK(HAL_I2S_DMAResume(&g_i2s2));
}

void player_set_SPK_val(uint8_t val)
{
    WM8978_SPKvol_Set(val);
}

void player_set_HP_val(uint8_t Lval, uint8_t Rval)
{
    WM8978_HPvol_Set(Lval, Rval);
}

void I2S_DMA_M1TransferCpltCallback(DMA_HandleTypeDef *hdma)
{
    if (xQueueReceiveFromISR(QueuePlayerReceive, buf1, NULL) != pdTRUE)
    {
        /* 发送事件 */
        player_polling_callback();
    }
}

void I2S_DMA_M0TransferCpltCallback(DMA_HandleTypeDef *hdma)
{
    if (xQueueReceiveFromISR(QueuePlayerReceive, buf0, NULL) != pdTRUE)
    {
        player_polling_callback();
    }
}

void I2S_DMA_TransferErrorCallback(DMA_HandleTypeDef *hdma)
{
    HAL_CHECK(HAL_UART_Transmit(&g_uart1, "i2s dma error\r\n", 15, 100));
}
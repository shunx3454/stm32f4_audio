#include "periph.h"
#include "audio.h"
#include "wm8978/wm8978.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "ff.h"
#include "decoder.h"
#include "player.h"
#include "bord_config.h"

#include <stdio.h>
#include <string.h>

QueueHandle_t QueuePCMData;
EventGroupHandle_t EventsAudio;
static StaticQueue_t xStaticQueue;
static uint8_t pucQueueStorageBuffer[I2S_DMA_QUEUE_SIZE * I2S_DMA_QUEUE_LENGTH] __ATTR_EXSRAM __ALIGNED(4);
static TaskHandle_t ThreadMusicDecode;

static decoder_t decoder;

static FIL music_fl;

void thread_music_decode(void *param);
AUDIO_EVENTS audio_wait_events(void);
DECODE_TYPE find_decoder_type(const char *);
uint32_t decoder_input_cb(void *dec, uint8_t *pbuf, uint32_t size);
void decoder_output_cb(void *dec, uint8_t *pbuf, uint32_t size);
void decoder_info_cb(void *dec);
void player_polling_cb(void);

int AudioInit(Audio *audio)
{
    /* config wm8978 */
    if (WM8978_Init() != 0)
    {
        printf("wm8978 error\r\n");
        return AR_HARDERROR;
    }

    WM8978_ADDA_Cfg(1, 0);
    WM8978_Input_Cfg(0, 0, 0);
    WM8978_Output_Cfg(1, 0);
    WM8978_HPvol_Set(0, 0);
    WM8978_SPKvol_Set(10);
    WM8978_I2S_Cfg(2, 0);

    /* 创建队列缓冲区 */
    QueuePCMData = xQueueCreateStatic(I2S_DMA_QUEUE_LENGTH, I2S_DMA_QUEUE_SIZE, pucQueueStorageBuffer, &xStaticQueue);

    /* 创建MP3解码任务 */
    xTaskCreate(thread_music_decode, "music decode", 1024, NULL, 2, &ThreadMusicDecode);
    vTaskSuspend(ThreadMusicDecode);

    /* 创建事件组 */
    if ((EventsAudio = xEventGroupCreate()) == NULL)
    {
        SYS_ERR("xEventGroupCreate");
        return AR_DECODERNOINIT_ERR;
    }

    player_init(&QueuePCMData, player_polling_cb);

    return AR_OK;
}

int AudioDelete(Audio *audio)
{
    player_stop();
    vTaskSuspend(ThreadMusicDecode);
    vTaskDelete(ThreadMusicDecode);
    vQueueDelete(QueuePCMData);
    return AR_OK;
}

void audio_loop(void)
{
    /*
        1.检测i2s 状态
     */
    audio_wait_events();
}

void thread_music_decode(void *param)
{
    for (;;)
    {
        decoder_loop(&decoder);
    }
}

/* 读取音乐文件，设置硬件播放器参数 */
int audio_start(const char *music_path)
{
    /*
        1.读取文件类型
        2.初始化解码器
            输入流回调，输出流回调，读取信息回调
        3.开启解码器
     */

    { /* music file prepare */
        /* open music file */
        if (music_fl.obj.fs != NULL)
        {
            f_close(&music_fl);
        }

        if (f_open(&music_fl, music_path, FA_READ))
        {
            SYS_ERR("f_open");
            return AR_FILEIO_ERR;
        }
    }

    // 关闭播放器和任务
    player_stop();
    vTaskSuspend(ThreadMusicDecode);

    // 初始化解码器
    decoder_init(&decoder, find_decoder_type(music_path), decoder_input_cb, decoder_output_cb, decoder_info_cb);

    // 开启播放器和任务
    vTaskResume(ThreadMusicDecode);
    vTaskDelay(1000);

    return AR_OK;
}

AUDIO_EVENTS audio_wait_events(void)
{
    return AUDIO_EVENT_ERROR;
}

uint32_t decoder_input_cb(void *dec, uint8_t *pbuf, uint32_t size)
{
    UINT nRead;
    if (f_read(&music_fl, pbuf, size, &nRead))
    {
        return 0;
    }
    return nRead;
}

void decoder_output_cb(void *dec, uint8_t *pbuf, uint32_t size)
{
    static uint32_t remians = 0;
    static uint8_t queue_buf[I2S_DMA_QUEUE_SIZE] __ATTR_EXSRAM __ALIGNED(4);
    static uint8_t *ptr;
    decoder_t *pdec = (decoder_t *)dec;

    switch (pdec->de_type)
    {
    case DECODER_TYPE_MP3:
        size *= (2 * pdec->chanals);
        break;
    case DECODER_TYPE_FLAC:
        if (pdec->bits_per_sample == 16)
        {
            uint16_t data16;
            for (uint32_t i = 0; i < size; i++)
            {
                data16 = *(uint16_t*)&(pbuf[i*4+2]);
                ((uint16_t *)pbuf)[i] = data16;
            }
            size *= 2;
        }
        else if (pdec->bits_per_sample == 24)
        {
            size *= 4;
        }
        else if (pdec->bits_per_sample == 32)
        {
            size *= 4;
        }
        break;
    default:
        break;
    }

    /* remians < I2S_DMA_QUEUE_SIZE always */

    if (remians + size < I2S_DMA_QUEUE_SIZE)
    {
        memmove(queue_buf + remians, pbuf, size);
        remians += size;
    }
    else
    {
        memmove(queue_buf + remians, pbuf, I2S_DMA_QUEUE_SIZE - remians);
        xQueueSendToBack(QueuePCMData, queue_buf, portMAX_DELAY);
        size -= (I2S_DMA_QUEUE_SIZE - remians);
        ptr = pbuf + (I2S_DMA_QUEUE_SIZE - remians);
        while (size >= I2S_DMA_QUEUE_SIZE)
        {
            xQueueSendToBack(QueuePCMData, ptr, portMAX_DELAY);
            ptr += I2S_DMA_QUEUE_SIZE;
            size -= I2S_DMA_QUEUE_SIZE;
        }
        remians = size;
        memmove(queue_buf, ptr, remians);
    }
}

void decoder_info_cb(void *dec)
{
    decoder_t *pdec = (decoder_t *)dec;
    printf("bits_per_sample: %d\r\n", pdec->bits_per_sample);
    printf("chanals: %d\r\n", pdec->chanals);
    printf("sample_rate: %ld\r\n", pdec->sample_rate);
    player_config(pdec->bits_per_sample, pdec->sample_rate);
    player_begin();
}

DECODE_TYPE find_decoder_type(const char *path)
{
    uint8_t pos = 0;
    char fixc[16];
    for (size_t i = 0; i < strlen(path); i++)
    {
        if (path[i] == '.')
        {
            pos = i;
            break;
        }
    }
    memset(fixc, 0, sizeof(fixc));
    strcpy(fixc, &path[pos + 1]);
    if (strcmp(fixc, "mp3") == 0)
    {
        return DECODER_TYPE_MP3;
    }
    else if (strcmp(fixc, "flac") == 0)
    {
        return DECODER_TYPE_FLAC;
    }
    else if (strcmp(fixc, "wav") == 0)
    {
        return DECODER_TYPE_WAV;
    }
    else
        return DECODER_TYPE_NONE;
}

void player_polling_cb(void)
{
    // printf("player polling\r\n");
}
#include "decoder.h"
#include <stdio.h>
#include "bord_config.h"
#include <string.h>
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include "flac.h"
#include "FreeRTOS.h"
#include "task.h"

/* common buffer */
uint8_t inbuf[DECODER_INBUF_SIZE] __ATTR_EXSRAM __ALIGNED(4);
uint8_t pcmbuf[DECODER_OUTBUF_SIZE] __ATTR_EXSRAM __ALIGNED(4);

/* minimp3 define */
mp3dec_t mp3_de;
mp3dec_frame_info_t mp3_de_info;

/* libfoxenflac define */
#define MAX_DECODER_SIZE (1024 * 150U)
#define MAX_BLOCK_SIZE (16384U)
#define MAX_CHANALS (2U)
uint8_t mem_flac[MAX_DECODER_SIZE] __ATTR_EXSRAM __ALIGNED(4);
fx_flac_t *flac_de;
uint32_t n_in;
uint32_t n_out;

void decoder_type_init(DECODE_TYPE type);
void de_mp3(decoder_t *dec);
void de_flac(decoder_t *dec);
void de_wave(decoder_t *dec);

void decoder_init(decoder_t *decoder, DECODE_TYPE type, decoder_input_t in, decoder_output_t out, decoder_info_callback_t info_cb)
{
    decoder->input = in;
    decoder->output = out;
    decoder->info_cb = info_cb;
    decoder->de_type = type;

    decoder->process_data.current_ptr = inbuf;
    decoder->process_data.eof = 0;
    decoder->process_data.remain = 0;
    decoder->process_data.end = 0;

    decoder_type_init(decoder->de_type);
}

void decoder_loop(decoder_t *dec)
{
    switch (dec->de_type)
    {
    case DECODER_TYPE_MP3:
        de_mp3(dec);
        break;
    case DECODER_TYPE_FLAC:
        de_flac(dec);
        break;
    case DECODER_TYPE_WAV:
        break;
    default:
        printf("Decoder file error\r\n");
        break;
    }
    if (dec->process_data.end)
    {
        vTaskSuspend(NULL);
    }
}

void decoder_type_init(DECODE_TYPE type)
{
    switch (type)
    {
    case DECODER_TYPE_MP3:
        /* minimp3 init */
        memset(&mp3_de, 0, sizeof(mp3dec_t));
        mp3dec_init(&mp3_de);
        break;
    case DECODER_TYPE_FLAC:
        flac_de = fx_flac_init(mem_flac, MAX_BLOCK_SIZE, MAX_CHANALS);
    default:
        break;
    }
}

void de_mp3(decoder_t *dec)
{
    if (dec->process_data.remain < DECODER_INPUT_THRESHOLD && !dec->process_data.eof)
    {
        if (dec->process_data.remain)
            memmove(inbuf, dec->process_data.current_ptr, dec->process_data.remain);
        dec->process_data.remain += dec->input(dec, inbuf + dec->process_data.remain, (DECODER_INBUF_SIZE - dec->process_data.remain));
        if (dec->process_data.remain < DECODER_INBUF_SIZE)
            dec->process_data.eof = 1;
        dec->process_data.current_ptr = inbuf;
    }

    int samples = mp3dec_decode_frame(&mp3_de, dec->process_data.current_ptr, dec->process_data.remain, (mp3d_sample_t *)pcmbuf, &mp3_de_info);
    dec->process_data.current_ptr += mp3_de_info.frame_bytes;
    dec->process_data.remain -= mp3_de_info.frame_bytes;

    if (samples)
    {
        dec->total_samples += samples;

        if (!dec->process_data.is_info_cb)
        {
            dec->bits_per_sample = 16;
            dec->chanals = mp3_de_info.channels;
            dec->sample_rate = mp3_de_info.hz;

            dec->info_cb(dec);

            dec->process_data.is_info_cb = 1;
        }
        dec->output(dec, pcmbuf, samples);
    }
    if (dec->process_data.eof && samples == 0)
    {
        dec->process_data.end = 1;
    }
}

void de_flac(decoder_t *dec)
{
    if (dec->process_data.remain < DECODER_INPUT_THRESHOLD && !dec->process_data.eof)
    {
        if (dec->process_data.remain)
        {
            for (uint32_t i = 0; i < dec->process_data.remain; i++)
                inbuf[i] = dec->process_data.current_ptr[i];
        }
        dec->process_data.remain += dec->input(dec, inbuf + dec->process_data.remain, (DECODER_INBUF_SIZE - dec->process_data.remain));
        if (dec->process_data.remain < DECODER_INBUF_SIZE)
            dec->process_data.eof = 1;
        dec->process_data.current_ptr = inbuf;
    }

    n_in = dec->process_data.remain;
    n_out = DECODER_OUTBUF_SIZE / 4;
    
    fx_flac_state_t status = fx_flac_process(flac_de, inbuf, &n_in, (int32_t *)pcmbuf, &n_out);

    dec->process_data.remain -= n_in;
    dec->process_data.current_ptr += n_in;

    if (status == FLAC_END_OF_METADATA)
    {
        if (!dec->process_data.is_info_cb)
        {
            dec->bits_per_sample = fx_flac_get_streaminfo(flac_de, FLAC_KEY_SAMPLE_SIZE);
            dec->chanals = fx_flac_get_streaminfo(flac_de, FLAC_KEY_N_CHANNELS);
            dec->sample_rate = fx_flac_get_streaminfo(flac_de, FLAC_KEY_SAMPLE_RATE);
            dec->info_cb(dec);
            dec->process_data.is_info_cb = 1;
        }
    }
    else if (status == FLAC_ERR)
    {
        printf("flac decoder err\r\n");
    }

    if (n_out)
    {
        dec->total_samples += (n_out / 2);
        dec->output(dec, pcmbuf, n_out);
    }

    if (n_in == 0 && n_out == 0 && dec->process_data.eof)
    {
        dec->process_data.end = 1;
    }
}
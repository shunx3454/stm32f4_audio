#ifndef __BSP_DECODER_H_
#define __BSP_DECODER_H_

#include <stdint.h>

#define MAX_DECODE_TYPE 10

typedef enum
{
    DECODER_TYPE_NONE = 0,
    DECODER_TYPE_MP3 = 1,
    DECODER_TYPE_FLAC = 2,
    DECODER_TYPE_WAV = 3,
} DECODE_TYPE;

typedef struct{
    uint32_t remain;
    uint8_t *current_ptr;
    uint8_t end;
    uint8_t eof;
    uint8_t is_info_cb;
}decoder_process_data_t;

typedef uint32_t (*decoder_input_t)(void *dec, uint8_t*pbuf, uint32_t size);
typedef void (*decoder_output_t)(void*dec, uint8_t*pbuf, uint32_t size);
typedef void (*decoder_info_callback_t)(void *dec);

typedef struct _decoder_t{
    uint8_t bits_per_sample;
    uint32_t sample_rate;
    uint32_t total_samples;
    uint8_t chanals;
    decoder_input_t input;
    decoder_output_t output;
    decoder_info_callback_t info_cb;
    DECODE_TYPE de_type;
    decoder_process_data_t process_data;
}decoder_t;


void decoder_init(decoder_t *decoder, DECODE_TYPE type, decoder_input_t in, decoder_output_t out, decoder_info_callback_t info_cb);
void decoder_loop(decoder_t *dec);

#endif
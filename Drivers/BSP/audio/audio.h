#ifndef __AUDIO_
#define __AUDIO_

#include "main.h"

/* AUDIO WORK STATES */
#define AUDIO_NOINIT 0
#define AUDIO_READY 1
#define AUDIO_STOP 2
#define AUDIO_PLAY 3
#define AUDIO_PLAY_POLL 4
#define AUDIO_ERROR 5

/* AUDIO API RESULT */
#define AR_OK 0
#define AR_MEM_ERR 1
#define AR_HARDERROR 2
#define AR_FILEIO_ERR 3
#define AR_MP3DECODER_ERR 4
#define AR_DECODERNOINIT_ERR 5
#define AR_NOINIT_ERR 6
#define AR_HARD_ERR 7
#define AR_TRANS_ERR 8

/* Audio class */
struct _Audio
{
    int (*getmp3info)(char *);
};

typedef struct _Audio Audio;

typedef enum {
    AUDIO_EVENT_ERROR = -1,
    AUDIO_EVENT_POLLING = 1,
    AUDIO_EVENT_CHANG = 2,
    AUDIO_EVENT_STOP = 3,
}AUDIO_EVENTS;


int AudioInit(Audio *audio);
int AudioDelete(Audio *audio);
int audio_start(const char *fmp3Path);
void audio_loop(void);

#endif

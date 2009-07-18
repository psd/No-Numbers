#ifndef WAV_H
#define WAV_H

/*
 *  handle
 */
typedef void *wav_t;

typedef struct wav_riff_chunk
{
	char riff[5];
	long packlen;
	char wave[5];
}
wav_riff_chunk_t;

/*
 *  open wav file
 */
extern void *wav_open(const char *filename, int test);

/*
 *  cycle through samples
 */
extern int wav_next(wav_t wav, char *left, char *right, size_t len);

/*
 *  close wav file
 */
extern int wav_close(void *p);


#endif

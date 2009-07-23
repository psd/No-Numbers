/*
 *   simple wav file reader
 *
 *   Copyright (c) Paul Downey (@psd), Andrew Back (@9600) 2009
 *
 *   http://github.com/psd/MrNo/tree/master
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include "base.h"
#include "wav.h"

struct wav_t
{
	char *filename;
	int test;
	FILE *fp;
	size_t head_len;
	unsigned long sample_rate;
	unsigned long bytes_per_second;
	unsigned long bytes_per_sample;
	unsigned long bits_per_sample;
	size_t chunk_len;
	size_t chunk_pos;
	void *chunk;
};


/*
 *  decode little-endian number
 */
static unsigned long wav_decode_ulong(const char *name, void *buff, int offset, int len)
{
unsigned char *p = buff;
unsigned long n = 0L;
int i = len;

	p += offset;
	while (i > 0) {
		n <<= 8;
		i--;
		n += (unsigned long)p[i];
	}

	if (verbose)
		fprintf(stderr, "%s: %0*lx (%lu)\n", name, len*2, n, n);

	return n;
}


/*
 *
	0000000      4952    4646    9be4    0007    4157    4556    6d66
		R   I   F   F 344 233  \a  \0   W   A   V   E  

	0 - 3 "RIFF" (ASCII)
	4 - 7 length of following package (little endian)
	8 - 11 "WAVE" (ASCII)
 *
 */
static int wav_read_riff(struct wav_t *wav)
{
char buff[16];

	if (1 != fread(buff, 12, 1, wav->fp)) {
		fprintf(stderr, "failed to read WAV RIFF header %s\n", wav->filename);
		return FAIL;
	}

	if (strncmp(buff, "RIFF", 4)) {
		fprintf(stderr, "RIFF string missing from WAV header %s\n", wav->filename);
		return FAIL;
	}

	if (strncmp(buff+8, "WAVE", 4)) {
		fprintf(stderr, "WAVE string missing from WAV header %s\n", wav->filename);
		return FAIL;
	}

	wav->head_len = wav_decode_ulong("header length", buff, 4, 4);

	return OK;
}


/*
	0000000      4952    4646    9be4    0007    4157    4556    6d66    2074
		R   I   F   F 344 233  \a  \0   W   A   V   E   f   m   t    
	0000020      0010    0000    0001    0002    ac44    0000    b110    0002
		020  \0  \0  \0 001  \0 002  \0   D 254  \0  \0 020 261 002  \0
	0000040      0004    0010    6164    6174    9bc0    0007    0000    0000
		004  \0 020  \0   d   a   t   a 300 233  \a  \0  \0  \0  \0  \0

	FORMAT Chunk (24 bytes in length total)
	0 - 3 "fmt_" (ASCII Characters)
	4 - 7 Length Of FORMAT Chunk (Binary, always 0x10)
	8 - 9 Always 0x01
	10 - 11 Channel Numbers (Always 0x01=Mono, 0x02=Stereo)
	12 - 15 Sample Rate (Binary, in Hz)
	16 - 19 Bytes Per Second
	20 - 21 Bytes Per Sample: 1=8 bit Mono, 2=8 bit Stereo or 16 bit Mono, 4=16 bit Stereo
	22 - 23 Bits Per Sample
*/
static int wav_read_format(struct wav_t *wav)
{
char buff[32];
unsigned long n;

	if (1 != fread(buff, 24, 1, wav->fp)) {
		fprintf(stderr, "failed to read WAV format header %s\n", wav->filename);
		return FAIL;
	}

	if (strncmp(buff, "fmt ", 4)) {
		fprintf(stderr, "fmt string missing from WAV header %s\n", wav->filename);
		return FAIL;
	}

	if (0x01 != (n = wav_decode_ulong("format length", buff, 8, 2))) {
		fprintf(stderr, "format length unexpected length: %lx\n", n);
		return FAIL;
	}

	if (0x02 != (n = wav_decode_ulong("format channels", buff, 10, 2))) {
		fprintf(stderr, "format unexpected number of channels: %lx\n", n);
		return FAIL;
	}

	wav->sample_rate = wav_decode_ulong("format sample rate", buff, 12, 4);
	wav->bytes_per_second = wav_decode_ulong("format bytes per second", buff, 16, 4);
	wav->bytes_per_sample = wav_decode_ulong("format bytes per sample", buff, 20, 2);
	wav->bits_per_sample = wav_decode_ulong("format bits per sample", buff, 22, 2);

	return OK;
}


/*
	0000040      0004    0010    6164    6174    9bc0    0007    0000    0000
		004  \0 020  \0   d   a   t   a 300 233  \a  \0  \0  \0  \0  \0
	0000060      0000    0000    0000    0000    0000    0000    0000    0000
		\0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0

	DATA Chunk
	0 - 3 "data" (ASCII Characters)
	4 - 7 Length Of Data To Follow
	8 - end Data (Samples)
*/
static int wav_read_chunk(struct wav_t *wav)
{
char buff[16];

	if (verbose)
		fprintf(stderr, "reading a chunk ..\n");

	if (1 != fread(buff, 8, 1, wav->fp)) {
		if (feof(wav->fp)) {
			if (verbose)
				fprintf(stderr, "end of file\n");
			return DONE;
		}
		fprintf(stderr, "failed to read WAV data chunk header %s\n", wav->filename);
		return FAIL;
	}

	if (strncmp(buff, "data", 4)) {
		fprintf(stderr, "data string missing from WAV data chunk %s\n", wav->filename);
		return FAIL;
	}

	wav->chunk_len = wav_decode_ulong("data chunk length", buff, 4, 4);

	if (NULL != wav->chunk)
		free(wav->chunk);

	if (NULL == (wav->chunk = malloc(wav->chunk_len+1))) {
		fprintf(stderr, "malloc wav failed: %s", strerror(errno));
		return FAIL;
	}

	if (1 != fread(wav->chunk, wav->chunk_len, 1, wav->fp)) {
		fprintf(stderr, "failed to read WAV data chunk %s", wav->filename);
		return FAIL;
	}

	wav->chunk_pos = 0;

	return OK;
}


/*
 *  cycle through samples
 *  - returns DONE when at end of file
 */
int wav_next(wav_t p, char *buff_left, char *buff_right, size_t len)
{
int status;
struct wav_t *wav = p;
unsigned long sample_left;
unsigned long sample_right;

	if (wav->test) {
		return DONE;
	}

	/*
	 *  move through file
	 */
	if (0 == wav->chunk_pos) {
		if (OK != (status = wav_read_chunk(wav)))
			return status;
	}

	sample_left = wav_decode_ulong("sample left", wav->chunk, wav->chunk_pos, 2);
	sample_right = wav_decode_ulong("sample right", wav->chunk, wav->chunk_pos + 2, 2);
	wav->chunk_pos += 4;

	if (wav->chunk_pos > wav->chunk_len) {
		wav->chunk_pos = 0L;
		fprintf(stderr, "end of chunk\n");
	}

	sprintf(buff_left, "%8.8lu\n", sample_left);
	sprintf(buff_right, "%8.8lu\n", sample_right);
	return OK;
}


/*
 *  open a WAV file
 */
static int wav_open_file(struct wav_t *wav)
{
	if (verbose)
		fprintf(stderr, "opening wav %s\n", wav->filename);

	if (NULL == (wav->fp = fopen(wav->filename, "r"))) {
		fprintf(stderr, "failed to open %s: %s\n", wav->filename, strerror(errno));
		return FAIL;
	}

	if (wav_read_riff(wav)) 
		return FAIL;

	if (wav_read_format(wav)) 
		return FAIL;

	return OK;
}


/*
 *  open a wav file
 */
wav_t wav_open(const char *filename, int test)
{
struct wav_t *wav;

	if (NULL == (wav = malloc(sizeof(struct wav_t)))) {
		fprintf(stderr, "malloc wav failed: %s\n", strerror(errno));
		return NULL;
	}

	wav->filename = strdup(filename);
	wav->test = test;
	wav->chunk = NULL;
	wav->chunk_len = 0;
	wav->chunk_pos = 0;

	if (test) 
		return OK;

	if (wav_open_file(wav))
		return NULL;

	return wav;
}


/*
 *  close a wav file
 */
int wav_close(wav_t wav)
{
	if (verbose) 
		fprintf(stderr, "closing wav\n");

	return OK;
}

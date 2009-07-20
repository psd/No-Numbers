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
	FILE *fp;
	size_t head_len;
	int test;
};


static unsigned long wav_decode_ulong(void *buff, int offset, int len)
{
unsigned char *p = buff;
unsigned long n = 0L;

	p += offset;
	while (len > 0) {
		n <<= 8;
		n += p[len];
		len--;
	}
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
		fprintf(stderr, "failed to read WAV RIFF header %s", wav->filename);
		return FAIL;
	}

	if (strncmp(buff, "RIFF", 4)) {
		fprintf(stderr, "RIFF string missing from WAV header %s", wav->filename);
		return FAIL;
	}

	if (strncmp(buff+8, "WAVE", 4)) {
		fprintf(stderr, "WAVE string missing from WAV header %s", wav->filename);
		return FAIL;
	}

	wav->head_len = wav_decode_ulong(buff, 4, 4);

	return OK;
}


/*
 *  open a wav file
 */
wav_t wav_open(const char *filename, int test)
{
struct wav_t *wav;
FILE *fp = NULL;

	if (!test) {
		if (verbose)
			fprintf(stderr, "opening wav %s\n", filename);

		if (NULL == (fp = fopen(filename, "r"))) {
			fprintf(stderr, "failed to open %s: %s", filename, strerror(errno));
			return NULL;
		}
	}
		
	if (NULL == (wav = malloc(sizeof(struct wav_t)))) {
		fprintf(stderr, "malloc wav failed: %s", strerror(errno));
		return NULL;
	}

	wav->fp = fp;
	wav->filename = strdup(filename);
	wav->test = test;

	if (wav_read_riff(wav)) {
		return NULL;
	}

	return wav;
}


/*
0000000      4952    4646    9be4    0007    4157    4556    6d66    2074
           R   I   F   F 344 233  \a  \0   W   A   V   E   f   m   t    
0000020      0010    0000    0001    0002    ac44    0000    b110    0002
         020  \0  \0  \0 001  \0 002  \0   D 254  \0  \0 020 261 002  \0
0000040      0004    0010    6164    6174    9bc0    0007    0000    0000
         004  \0 020  \0   d   a   t   a 300 233  \a  \0  \0  \0  \0  \0
0000060      0000    0000    0000    0000    0000    0000    0000    0000
          \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0

FORMAT Chunk (24 bytes in length total)
Byte Number
0 - 3
"fmt_" (ASCII Characters)
4 - 7
Length Of FORMAT Chunk (Binary, always 0x10)
8 - 9
Always 0x01
10 - 11
Channel Numbers (Always 0x01=Mono, 0x02=Stereo)
12 - 15
Sample Rate (Binary, in Hz)
16 - 19
Bytes Per Second
20 - 21
Bytes Per Sample: 1=8 bit Mono, 2=8 bit Stereo or 16 bit Mono, 4=16 bit Stereo
22 - 23
Bits Per Sample


DATA Chunk
Byte Number
0 - 3
"data" (ASCII Characters)
4 - 7
Length Of Data To Follow
8 - end
Data (Samples)
*/

/*
 *  cycle through samples
 */
int wav_next(wav_t p, char *buff_left, char *buff_right, size_t len)
{
struct wav_t *wav = p;
long sample;

	if (wav->test) {
		return DONE;
	}
	sample = labs(((long)rand() * (long)rand()) % 100000000L);

	sprintf(buff_left, "%8.8ld\n", sample);
	sprintf(buff_right, "%8.8ld\n", sample);

	return OK;
}

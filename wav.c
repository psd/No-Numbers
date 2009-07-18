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


/*
 *  open a wav file
 */
wav_t wav_open(const char *filename, int test)
{
FILE *fp;

	if (test)
		return stdin;

	if (verbose)
		fprintf(stderr, "opening wav %s\n", filename);

	if (NULL == (fp = fopen(filename, "r"))) {
		fprintf(stderr, "failed to open %s: %s", filename, strerror(errno));
		return NULL;
	}

	return fp;
}

/*
Byte Number
0 - 3
"RIFF" (ASCII Characters)
4 - 7
Total Length Of Package To Follow (Binary, little endian)
8 - 11
"WAVE" (ASCII Characters)

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
int wav_next(wav_t wav, char *buff_left, char *buff_right, size_t len)
{
long sample;

	sample = labs(((long)rand() * (long)rand()) % 100000000L);

	sprintf(buff_left, "%8.8ld\n", sample);
	sprintf(buff_right, "%8.8ld\n", sample);

	return OK;
}

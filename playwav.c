/*
 *   cycle through a WAV file, outputting each stereo sample as a pair of 8 digit numbers to a pair of serial ports
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

#include "base.h"
#include "wav.h"
#include "tty.h"


/* 
 *  options
 */
static const char *left_tty = "/dev/ttys000";
static const char *right_tty = "/dev/ttys001";
static const char *filename = "in.wav";

static useconds_t interval = 1000000L;
static useconds_t delay = 10000L;

static int test = 0;
static int stty = 1;
int verbose = 1;


/* 
 *  command line usage
 */
static void usage(const char *command) 
{
	fprintf(stderr, "usage: %s [opts]\n", command);
	fprintf(stderr, "\n");
	fprintf(stderr, "   -t              test mode - ignores file and cycles through some numbers\n");
	fprintf(stderr, "   -s              silent mode\n");
	fprintf(stderr, "   -n              non-tty mode - don't assert stty settings\n");
	fprintf(stderr, "   -h              print this help\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "   -f filename     wav file to decode - %s\n", filename);
	fprintf(stderr, "   -l left_tty     pathname for left-channel output - %s\n", left_tty);
	fprintf(stderr, "   -r right_tty    pathname for right-channel output - %s\n", right_tty);
	fprintf(stderr, "   -i useconds     interval between outputting a sample in useconds\n");
	fprintf(stderr, "   -d useconds     interval between outputting each character in useconds\n");
	return;
}


/*
 *  parse command line options
 */
static int getoptions(int argc, char *argv[])
{
int c;
extern char *optarg;

	while ((c = getopt(argc, argv, "tsnh?l:r:f:i:d:")) != -1) {
		switch(c) {
		case 't':
			test = 1;
			break;
		case 's':
			verbose = 0;
			break;
		case 'n':
			stty = 0;
			break;
		case 'l':
			left_tty = optarg;
			break;
		case 'r':
			right_tty = optarg;
			break;
		case 'f':
			filename = optarg;
			break;
		case 'i':
			interval = strtoll(optarg, NULL, 10);
			break;
		case 'd':
			delay = strtoll(optarg, NULL, 10);
			break;
		case '?':
		case 'h':
		default:
			usage(argv[0]);
			return FAIL;
		}
	}

	if(verbose) { 
		fprintf(stderr, "interval: %lld microseconds\n", (long long)interval);
	}

	return OK;
}


static int playwav()
{
wav_t wav;
tty_t tty_left;
tty_t tty_right;
char buff_left[16];
char buff_right[16];
int countdown = 10;

	if (NULL == (wav = wav_open(filename, test)))
		return FAIL;

	if (NULL == (tty_left = tty_open(left_tty, stty, delay)))
		return FAIL;

	if (NULL == (tty_right = tty_open(right_tty, stty, delay)))
		return FAIL;

	for (;;) {

		if (countdown) {
			countdown--;
			memset(buff_left, '0'+countdown, 8);
			memset(buff_right, '0'+countdown, 8);
		}
		else {
			switch (wav_next(wav, buff_left, buff_right, 8)) {
			case OK:
				break;
			case FAIL:
				return FAIL;
			default:
				countdown = 10;
				continue;
			}
		}

		if (tty_write(tty_left, buff_left, 8))
			return FAIL;

		if (tty_write(tty_right, buff_right, 8))
			return FAIL;

		usleep(interval);
	}

	return OK;
}

int main(int argc, char *argv[], char **envp)
{
	if (getoptions(argc, argv))
		exit(1);

	srand(time(NULL));
	
	if (playwav()) 
		exit(2);

	exit(0);
}

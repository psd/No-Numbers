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
#include <termios.h>

#ifndef OK
#define OK 0
#endif

#ifndef FAIL
#define FAIL -1
#endif


/* 
 *  options
 */
static const char *left_tty = "/dev/ttyu0";
static const char *right_tty = "/dev/ttyu1";
static const char *filename = "in.wav";
static useconds_t interval = 1000000L;
static int verbose = 1;
static int test = 0;
static int stty = 1;


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
	fprintf(stderr, "   -i useconds     interval between outputting numbers in useconds\n");
	return;
}


/*
 *  parse command line options
 */
static int getoptions(int argc, char *argv[])
{
int c;
extern char *optarg;

	while ((c = getopt(argc, argv, "tsnhl:r:f:i:")) != -1) {
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

/*
 *  open a tty, assert settings
 */
static int open_tty(const char *tty)
{
int fd;

	if (0 > (fd = open(tty, O_RDWR|O_NONBLOCK))) {
		fprintf(stderr, "failed to open %s: %s", tty, strerror(errno));
		return fd;
	}

	if (verbose) {
		fprintf(stderr, "opened tty %s as %d\n", tty, fd);
	}

	if (stty) {
		fprintf(stderr, "asserting stty settings ..\n");
	}

	return fd;
}

static void *open_wav(const char *filename)
{
FILE *fp;

	if (NULL == (fp = fopen(filename, "r"))) {
		fprintf(stderr, "failed to open %s: %s", filename, strerror(errno));
		return NULL;
	}

	if (verbose)
		fprintf(stderr, "opened wav %s as %p\n", filename, fp);

	return fp;
}

static int main_path()
{
void *wav;
int left;
int right;
long sample;
char buff[16];

	srand(time(NULL));

	if (0 > (left = open_tty(left_tty))
		|| 0 > (right = open_tty(right_tty)))
		return FAIL;

	if (!test) {
		if (NULL == (wav = open_wav(filename)))
			return FAIL;
	}

	for (;;) {
		sample = labs(((long)rand() * (long)rand()) % 100000000L);

		sprintf(buff, "%8.8ld", sample);

		write(left, buff, 8);
		write(right, buff, 8);
		write(1, buff, 8);

		//puts(buff);

		usleep(interval);
	}

	return OK;
}

int main(int argc, char *argv[], char **envp)
{
	if (getoptions(argc, argv))
		exit(1);
	
	if (main_path()) 
		exit(2);

	exit(0);
}

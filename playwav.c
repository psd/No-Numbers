/*
 *  cycle through a WAV file, outputting each sample as an 8 digit number to a serial port
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

int main(int argc, char **argv, char **envp)
{
const char *ttyname = "/dev/ttywf";
const char *filename = "in.wav";
int ttyfd;
FILE *fp;
int verbose = 1;
useconds_t interval = 1000000L;

int c;
extern char *optarg;
extern int optind, optopt, opterr;

	while ((c = getopt(argc, argv, "st:f:")) != -1) {
		switch(c) {
		case 's':
			verbose = 0;
			break;
		case 'f':
			ttyname = optarg;
			break;
		case 't':
			filename = optarg;
			break;
		case 'i':
			interval = strtoll(optarg, NULL, 10);
		case '?':
		case 'h':
		default:
			fprintf(stderr, "usage: %s [-sh?] [-i useconds] [-t ttyname] [-f filename]\n", basename(argv[0]));
			exit(2);
		}
	}

	if (0 > (ttyfd = open(ttyname, O_RDWR|O_NONBLOCK))) {
		fprintf(stderr, "failed to open %s: %s", ttyname, strerror(errno));
	}

	if(verbose) fprintf(stderr, "opened tty %s as %d\n", ttyname, ttyfd);

	if (NULL == (fp = fopen(filename, "r"))) {
		fprintf(stderr, "failed to open %s: %s", filename, strerror(errno));
	}

	if(verbose) fprintf(stderr, "opened wav %s as %p\n", filename, fp);
	if(verbose) fprintf(stderr, "interval: %lld microseconds\n", (long long)interval);

	exit(0);
}

/*
 *   handle serial ttys
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
#include <libgen.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include "base.h"
#include "tty.h"

struct tty_t
{
	char *filename;
	int fd;
	useconds_t delay;
};

/*
 *  assert terminal settings
 */
static int tty_stty(const char *ttyname, int fd)
{
struct termios termios;

	if (verbose)
		fprintf(stderr, "asserting stty settings ..\n");

	if (tcgetattr(fd, &termios)) {
		fprintf(stderr, "tcgetattr failed %s: %s", ttyname, strerror(errno));
		return FAIL;
	}

	cfmakeraw(&termios);

	/*
	 *  assert 8 bits, no parity, one stop bit
	 */
	termios.c_cflag &= ~(CSIZE | PARENB);
	termios.c_cflag |= (CS8 | CSTOPB);

	if (cfsetospeed(&termios, B9600)) {
		fprintf(stderr, "cfsetospeed failed %s: %s", ttyname, strerror(errno));
		return FAIL;
	}

	if (tcsetattr(fd, TCSAFLUSH, &termios)) {
		fprintf(stderr, "tcsetattr failed %s: %s", ttyname, strerror(errno));
		return FAIL;
	}

	return OK;
}


/*
 *  open a tty, assert settings
 */
tty_t tty_open(const char *ttyname, int stty, useconds_t delay)
{
struct tty_t *tty;
int fd;

	if (0 > (fd = open(ttyname, O_RDWR|O_NONBLOCK))) {
		fprintf(stderr, "failed to open %s: %s", ttyname, strerror(errno));
		return NULL;
	}

	if (verbose) 
		fprintf(stderr, "opened tty %s as %d\n", ttyname, fd);

	if (stty)
		if (tty_stty(ttyname, fd)) 
			return NULL;

	if (NULL == (tty = malloc(sizeof(struct tty_t)))) {
		fprintf(stderr, "malloc tty failed: %s", strerror(errno));
		return NULL;
	}

	tty->fd = fd;
	tty->filename = strdup(ttyname);
	tty->delay = delay;

	return tty;
}


/*
 *  write to a tty
 */
int tty_write(tty_t p, const char *buff, int len)
{
struct tty_t *tty = p;
int i;

	if (verbose)
		fprintf(stderr, "%s: ", tty->filename);

	if (tty->delay) {
		for (i = 0; i < len; i++) {
			if (1 != write(tty->fd, buff+i, 1)) {
				fprintf(stderr, "write failed %s: %s", tty->filename, strerror(errno));
				return FAIL;
			}
			if (verbose)
				write(2, buff+1, 1);
			usleep(tty->delay);
		}
	} else {
		if (len != write(tty->fd, buff, len)) {
			fprintf(stderr, "write failed %s: %s", tty->filename, strerror(errno));
			return FAIL;
		}
	
		if (verbose)
			write(2, buff, len);
	}
	if (verbose)
		write(2, "\n", 1);

	return OK;
}

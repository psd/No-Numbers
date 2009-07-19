#ifndef TTY_H
#define TTY_H

/*
 *  handle
 */
typedef void *tty_t;

/*
 *  open a tty, assert settings
 */
extern tty_t tty_open(const char *tty, int stty, useconds_t delay);

/*
 *  write to a tty
 */
extern int tty_write(tty_t p, const char *buff, int len);

#endif

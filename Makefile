CFLAGS=-Wall
all::	playwav

playwav::	playwav.o wav.o tty.o
	$(CC) $(CFLAGS) -o $@ playwav.o tty.o wav.o

tty.o::		tty.h base.h tty.c
wav.o::		wav.h base.h wav.c
playwav.o::	wav.h base.h playwav.c

clean::;	rm -f *.o core

#ifdef __GNUC__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include "riscv.h"

int sleep1ms(void)
{
	usleep(1000);
	return 1;
}

void capture_keyb(int capture)
{
	struct termios term;
	tcgetattr(0, &term);
	term.c_lflag = capture ?
		term.c_lflag & ~(ICANON | ECHO) :
		term.c_lflag |  (ICANON | ECHO);
	tcsetattr(0, TCSANOW, &term);
}

void console_init(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);

	capture_keyb(1);
}

void console_restore(void)
{
	capture_keyb(0);
	tcflush(0, TCIFLUSH);
}

int console_kbhit(void)
{
	int br;
	ioctl(0, FIONREAD, &br);
	return br > 0;
}

int console_getchar(void)
{
	char ch;
	read(0, &ch, 1);
	return ch;
}

void console_putchar(int ch)
{
	putchar(ch);
}

#endif

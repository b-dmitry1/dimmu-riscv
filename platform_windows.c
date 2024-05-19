#ifdef _WIN32

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "riscv.h"

int sleep1ms(void)
{
	int t = (int)GetTickCount();
	Sleep(1);
	return ((int)GetTickCount()) - t;
}

void console_init(void)
{
}

void console_restore(void)
{
}

int console_kbhit(void)
{
	return _kbhit();
}

int console_getchar(void)
{
	return _getch();
}

void console_putchar(int ch)
{
	putchar(ch);
}

#endif

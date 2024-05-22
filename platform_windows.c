#ifdef _WIN32

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "riscv.h"

// Функции в этом файле относятся к хост-платформе Windows

// Отправить поток в сон на 1 мс
int sleep1ms(void)
{
	// В Windows сон будет от 1 до 16 мс, вернуть количество пропущенных мс
	int t = (int)GetTickCount();
	Sleep(1);
	return ((int)GetTickCount()) - t;
}

void console_init(void)
{
	// Здесь ничего не нужно делать
}

void console_restore(void)
{
	// Здесь ничего не нужно делать
}

// Проверка нажатия клавиш
int console_kbhit(void)
{
	return _kbhit();
}

// Чтение кода нажатой клавиши
int console_getchar(void)
{
	return _getch();
}

// Вывод в TTY
void console_putchar(int ch)
{
	putchar(ch);
}

#endif

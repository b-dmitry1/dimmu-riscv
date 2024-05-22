#ifndef PLATFORM_H
#define PLATFORM_H

// Платформенные функции для облегчения портирования

int  sleep1ms();
void console_init(void);
void console_restore(void);
int  console_kbhit(void);
int  console_getchar(void);
void console_putchar(int ch);

#endif

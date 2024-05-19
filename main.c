#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "riscv.h"
#include "platform.h"

#define KERNEL_LOAD_OFFSET	0
#define DTB_LOAD_OFFSET		(sizeof(ram) - 65536)

uint8_t ram[RAM_SIZE];

riscv_t cpu;

// Загрузка файла в физическую память процессора
static int load_file(const char* name, int ram_offset)
{
	FILE* f;

	if (ram_offset < 0 || ram_offset >= sizeof(ram))
	{
		printf("\"%s\": load offset is out of range\n", name);
		return 0;
	}

	f = fopen(name, "rb");
	if (f == NULL)
	{
		printf("\"%s\": load error\n", name);
		return 0;
	}

	fread(&ram[ram_offset], 1, sizeof(ram) - ram_offset, f);

	fclose(f);

	return 1;
}

int main()
{
	// Инициализировать и сбросить ядро процессора
	memset(&cpu, 0, sizeof(cpu));
	cpu.ram = ram;
	reset(&cpu);

	// Загрузить ядро
	if (!load_file(IMAGE_FILE, KERNEL_LOAD_OFFSET))
		return 1;

	// Загрузить devicetree файл
	if (!load_file(DTB_FILE, DTB_LOAD_OFFSET))
		return 1;

	// Передать параметры ядру
	cpu.r[10] = 0; // Идентификатор ядра
	cpu.r[11] = RAM_START + DTB_LOAD_OFFSET; // Адрес devicetree
	// Точка входа
	cpu.pc = RAM_START + KERNEL_LOAD_OFFSET;

	// Инициализировать консольный ввод/вывод
	console_init();

	// Запланировать восстановление консольного ввода/вывода при выходе
	atexit(console_restore);

	// Запустить эмуляцию
	for (;;)
		step100(&cpu);

	return 0;
}

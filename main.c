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

// �������� ����� � ���������� ������ ����������
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
	// ���������������� � �������� ���� ����������
	memset(&cpu, 0, sizeof(cpu));
	cpu.ram = ram;
	reset(&cpu);

	// ��������� ����
	if (!load_file(IMAGE_FILE, KERNEL_LOAD_OFFSET))
		return 1;

	// ��������� devicetree ����
	if (!load_file(DTB_FILE, DTB_LOAD_OFFSET))
		return 1;

	// �������� ��������� ����
	cpu.r[10] = 0; // ������������� ����
	cpu.r[11] = RAM_START + DTB_LOAD_OFFSET; // ����� devicetree
	// ����� �����
	cpu.pc = RAM_START + KERNEL_LOAD_OFFSET;

	// ���������������� ���������� ����/�����
	console_init();

	// ������������� �������������� ����������� �����/������ ��� ������
	atexit(console_restore);

	// ��������� ��������
	for (;;)
		step100(&cpu);

	return 0;
}

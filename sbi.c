#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "riscv.h"
#include "sbi.h"
#include "platform.h"

static void sbi_ecall_base(riscv_t* cpu)
{
	switch (cpu->r[16])
	{
		case 0:
			// Номер спецификации SBI
			cpu->r[10] = 0;
			cpu->r[11] = 2;
			break;
		case 1:
			// Идентификатор SBI
			cpu->r[10] = 0;
			cpu->r[11] = 0x12345;
			break;
		case 2:
			// Номер версии SBI
			cpu->r[10] = 0;
			cpu->r[11] = 1;
			break;
		case 3:
			// Запрос кодов расширений
			cpu->r[11] = 0;
			switch (cpu->r[10])
			{
				case 0x54494D45: // Таймер RISC-V
					cpu->r[11] = 1; // Присутствует
					break;
			}
			cpu->r[10] = 0;
			break;
		case 4:
			// Идентификатор производителя (Vendor ID)
			cpu->r[10] = 0;
			cpu->r[11] = 0x12345;
			break;
		case 5:
			// Идентификатор архитектуры (Arch ID)
			cpu->r[10] = 0;
			cpu->r[11] = 0;
			break;
		case 6:
			// Аппаратный идентификатор (Machine implementation ID)
			cpu->r[10] = 0;
			cpu->r[11] = 0;
			break;
		default:
			// Прочие функции не поддерживаются
			cpu->r[10] = -2;
			cpu->r[11] = 0;
			break;
	}
}

static void sbi_ecall_timer(riscv_t* cpu)
{
	switch (cpu->r[16])
	{
		case 0:
			// Установка порога срабатывания таймера
#if XLEN == 64
			set_mtimecmp64(cpu, cpu->r[10]);
#else
			set_mtimecmp(cpu, cpu->r[11], cpu->r[10]);
#endif
			cpu->r[10] = 0;
			cpu->r[11] = 0;
			break;
		default:
			// Прочие функции не поддерживаются
			cpu->r[10] = -2;
			cpu->r[11] = 0;
			break;
	}
}

int sbi_ecall(riscv_t* cpu)
{
	switch (cpu->r[17])
	{
		case 0x00:
			// Установка порога срабатывания таймера
#if XLEN == 64
			set_mtimecmp64(cpu, cpu->r[10]);
#else
			set_mtimecmp(cpu, cpu->r[11], cpu->r[10]);
#endif
			cpu->r[10] = 0;
			cpu->r[11] = 0;
			return 1;
		case 0x01:
			// Вывод символа в консоль
			console_putchar(cpu->r[10]);
			cpu->r[10] = 0;
			cpu->r[11] = 0;
			return 1;
		case 0x02:
			// Чтение символа из консоли
			cpu->r[10] = -1;
			if (console_kbhit())
				cpu->r[10] = console_getchar();
			cpu->r[11] = 0;
			return 1;
		case 0x10:
			sbi_ecall_base(cpu);
			return 1;
		case 0x54494D45:
			sbi_ecall_timer(cpu);
			return 1;
	}
	return 0;
}

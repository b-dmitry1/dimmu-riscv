#include <stdint.h>
#include "riscv.h"

// Расширение "A": Атомарные инструкции
void do_atomic(riscv_t* cpu, uint32_t instr, int rs1, int rs2, int rd, int func3, int func7)
{
	ui addr;
	si value;
	int32_t a32, b32, res32;

	addr = cpu->r[rs1];
	if (func3 == 2)
	{
		// 32-битные атомарные инструкции
		// Здесь всего 1 ядро, и эмулятор не меняет порядок выполнения инструкций,
		// поэтому, чтобы было попроще, не будем проверять aq/rl флаги
		switch (func7 >> 2)
		{
			case 0x00: // AMOADD.W
				// Атомарное сложение
				// Прочитать значение по адресу из регистра rs1
				if (!read32(cpu, addr, &value))
					return;
				// Если успешно (нет сбоя чтения/page fault), то сложить
				// полученное значение со значением регистра rs2
				a32 = (int32_t)value;
				b32 = (int32_t)cpu->r[rs2];
				res32 = a32 + b32;
				// Записать значение по адресу из регистра rs1
				// Если успешно, то записать в регистр rd прочитанное ранее из памяти значение
				if (write32(cpu, addr, res32))
					cpu->r[rd] = a32;
				return;
			case 0x01: // AMOSWAP.W
				// Атомарный обмен
				if (!read32(cpu, addr, &value))
					return;
				a32 = (int32_t)value;
				b32 = (int32_t)cpu->r[rs2];
				res32 = b32;
				if (write32(cpu, addr, res32))
					cpu->r[rd] = a32;
				return;
			case 0x02: // LR.W
				// Чтение с резервированием адреса
				// Инструкция считывает слово по адресу, находящемуся в регистре rs1
				// и добавляет резервирование этого адреса для последующей инструкции SC.
				// Здесь всего 1 адрес используется, но в спецификации разрешено 1 ядру
				// иметь набор активных адресов
				if (addr & 3u)
				{
					// Слово должно быть выровнено в памяти
					trap(cpu, EX_STORE_MISALIGNED, addr);
					return;
				}
				// Попробовать прочитать значение
				if (!read32(cpu, addr, &value))
					return;
				// Если успешно, то записать результат в rd и добавить резервирование
				cpu->r[rd] = (int32_t)value;
				cpu->res_addr = addr;
				return;
			case 0x03: // SC.W
				// Запись по совпадению зарезервированного адреса
				// Инструкция работает в паре с LR
				if (addr & 3u)
				{
					// Слово должно быть выровнено в памяти
					trap(cpu, EX_STORE_MISALIGNED, addr);
					return;
				}

				// Проверить, совпал ли ранее зарезервированный адрес
				if (addr != cpu->res_addr)
				{
					// Если нет, то вернуть ненулевое значение
					cpu->r[rd] = 1;
					return;
				}

				// Попытаться записать
				if (write32(cpu, addr, (int32_t)cpu->r[rs2]))
					cpu->r[rd] = 0; // ноль - успешно
				else
					cpu->r[rd] = 1; // ненулевое значение - ошибка

				cpu->res_addr = ~0u; // Сбросить резервирование адреса
				return;
			case 0x04: // AMOXOR.W
				// Атомарная операция "исключающее ИЛИ"
				if (!read32(cpu, addr, &value))
					return;
				a32 = (int32_t)value;
				b32 = (int32_t)cpu->r[rs2];
				res32 = a32 ^ b32;
				if (write32(cpu, addr, res32))
					cpu->r[rd] = a32;
				return;
			case 0x08: // AMOOR.W
				// Атомарная операция "ИЛИ"
				if (!read32(cpu, addr, &value))
					return;
				a32 = (int32_t)value;
				b32 = (int32_t)cpu->r[rs2];
				res32 = a32 | b32;
				if (write32(cpu, addr, res32))
					cpu->r[rd] = a32;
				return;
			case 0x0C: // AMOAND.W
				// Атомарная операция "И"
				if (!read32(cpu, addr, &value))
					return;
				a32 = (int32_t)value;
				b32 = (int32_t)cpu->r[rs2];
				res32 = a32 & b32;
				if (write32(cpu, addr, res32))
					cpu->r[rd] = a32;
				return;
			case 0x10: // AMOMIN.W
				// Атомарная запись минимального значения со знаком
				if (!read32(cpu, addr, &value))
					return;
				a32 = (int32_t)value;
				b32 = (int32_t)cpu->r[rs2];
				res32 = a32 < b32 ? a32 : b32;
				if (write32(cpu, addr, res32))
					cpu->r[rd] = a32;
				return;
			case 0x14: // AMOMAX.W
				// Атомарная запись максимального значения со знаком
				if (!read32(cpu, addr, &value))
					return;
				a32 = (int32_t)value;
				b32 = (int32_t)cpu->r[rs2];
				res32 = a32 > b32 ? a32 : b32;
				if (write32(cpu, addr, res32))
					cpu->r[rd] = a32;
				return;
			case 0x18: // AMOMINU.W
				// Атомарная запись минимального значения без знака
				if (!read32(cpu, addr, &value))
					return;
				a32 = (int32_t)value;
				b32 = (int32_t)cpu->r[rs2];
				res32 = ((uint32_t)a32) < (uint32_t)b32 ? a32 : b32;
				if (write32(cpu, addr, res32))
					cpu->r[rd] = a32;
				return;
			case 0x1C: // AMOMAXU.W
				// Атомарная запись максимального значения без знака
				if (!read32(cpu, addr, &value))
					return;
				a32 = (int32_t)value;
				b32 = (int32_t)cpu->r[rs2];
				res32 = ((uint32_t)a32) > (uint32_t)b32 ? a32 : b32;
				if (write32(cpu, addr, res32))
					cpu->r[rd] = a32;
				return;
		}
	}
	else if (func3 == 3)
	{
		// 64-битные атомарные инструкции, выполняются аналогично 32-битным
		switch (func7 >> 2)
		{
			case 0x00: // AMOADD.D
				if (!read64(cpu, addr, &value))
					return;
				if (!write64(cpu, addr, value + cpu->r[rs2]))
					return;
				cpu->r[rd] = value;
				return;
			case 0x01: // AMOSWAP.D
				if (!read64(cpu, addr, &value))
					return;
				if (!write64(cpu, addr, cpu->r[rs2]))
					return;
				cpu->r[rd] = value;
				return;
			case 0x02: // LR.D
				if (addr & 7u)
				{
					trap(cpu, EX_STORE_MISALIGNED, addr);
					return;
				}
				if (!read64(cpu, addr, &value))
					return;
				cpu->r[rd] = value;
				cpu->res_addr = addr;
				return;
			case 0x03: // SC.D
				if (addr & 7u)
				{
					trap(cpu, EX_STORE_MISALIGNED, addr);
					return;
				}

				if (addr != cpu->res_addr)
				{
					cpu->r[rd] = 1;
					return;
				}

				if (write64(cpu, addr, cpu->r[rs2]))
					cpu->r[rd] = 0;
				else
					cpu->r[rd] = 1;

				cpu->res_addr = ~0u;
				return;
			case 0x04: // AMOXOR.D
				if (!read64(cpu, addr, &value))
					return;
				if (!write64(cpu, addr, value ^ cpu->r[rs2]))
					return;
				cpu->r[rd] = value;
				return;
			case 0x08: // AMOOR.D
				if (!read64(cpu, addr, &value))
					return;
				if (!write64(cpu, addr, value | cpu->r[rs2]))
					return;
				cpu->r[rd] = value;
				return;
			case 0x0C: // AMOAND.D
				if (!read64(cpu, addr, &value))
					return;
				if (!write64(cpu, addr, value & cpu->r[rs2]))
					return;
				cpu->r[rd] = value;
				return;
			case 0x10: // AMOMIN.D
				if (!read64(cpu, addr, &value))
					return;
				if (!write64(cpu, addr, value < cpu->r[rs2] ? value : cpu->r[rs2]))
					return;
				cpu->r[rd] = value;
				return;
			case 0x14: // AMOMAX.D
				if (!read64(cpu, addr, &value))
					return;
				if (!write64(cpu, addr, value > cpu->r[rs2] ? value : cpu->r[rs2]))
					return;
				cpu->r[rd] = value;
				return;
			case 0x18: // AMOMINU.D
				if (!read64(cpu, addr, &value))
					return;
				if (!write64(cpu, addr, ((uint64_t)value) < (uint64_t)cpu->r[rs2] ? value : cpu->r[rs2]))
					return;
				cpu->r[rd] = value;
				return;
			case 0x1C: // AMOMAXU.D
				if (!read64(cpu, addr, &value))
					return;
				if (!write64(cpu, addr, ((uint64_t)value) > (uint64_t)cpu->r[rs2] ? value : cpu->r[rs2]))
					return;
				cpu->r[rd] = value;
				return;
		}
	}

	trap(cpu, EX_INSTR_ILLEGAL, cpu->instr_pc);
}

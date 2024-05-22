#include "riscv.h"
#include "decode.h"

// Привилегированные инструкции
void do_priv(riscv_t* cpu, uint32_t instr, int rs1, int rs2, int rd, int func3, int func7)
{
	uint32_t csr;
	ui zimm;
	si value;

	// Код функции или номер регистра
	csr = bits(instr, 31, 20);
	// Беззнаковая константа для CSRR*I
	zimm = bits(instr, 19, 15);

	switch (func3)
	{
		case 0:
			switch (csr)
			{
				case 0x000:
					// ECALL - системный вызов.
					// Сначала проверить известные команды,
					// и если ни одна не подошла, то сгенерировать исключение
					// в зависимости от режима (пользователь или супервизор)
					if (!sbi_ecall(cpu))
						trap(cpu, cpu->s_mode ? EX_ECALL_S : EX_ECALL_U, 0);
					return;
				case 0x001:
					// EBREAK - вызов отладчика
					// Если сюда попали в Linux, то что-то определённо не так с эмулятором
					trap(cpu, EX_BREAKPOINT, 0);
					return;
				case 0x102:
					// SRET - возврат из исключения/прерывания
					sret(cpu);
					return;
				case 0x105:
					// WFI - спящий режим до появления прерывания
					cpu->wfi = 1;
					return;
				case 0x120:
					// SFENCE.VMA - очистка кэша трансляции адресов (здесь не используется)
					return;
			}
			break;
		case 1:
			// CSRRW: чтение-запись управляющего регистра
			value = rd == 0 ? 0 : csr_read(cpu, csr);
			csr_write(cpu, csr, cpu->r[rs1]);
			cpu->r[rd] = value;
			return;
		case 2:
			// CSRRS - чтение-установка битов управляющего регистра
			value = rd == 0 ? 0 : csr_read(cpu, csr);
			csr_write(cpu, csr, value | cpu->r[rs1]);
			cpu->r[rd] = value;
			return;
		case 3:
			// CSRRC - чтение-сброс битов управляющего регистра
			value = rd == 0 ? 0 : csr_read(cpu, csr);
			csr_write(cpu, csr, value & ~cpu->r[rs1]);
			cpu->r[rd] = value;
			return;
		case 5:
			// CSRRWI: чтение-запись управляющего регистра (zimm вместо rs1)
			value = rd == 0 ? 0 : csr_read(cpu, csr);
			csr_write(cpu, csr, zimm);
			cpu->r[rd] = value;
			return;
		case 6:
			// CSRRSI: чтение-установка битов управляющего регистра (zimm вместо rs1)
			value = rd == 0 ? 0 : csr_read(cpu, csr);
			csr_write(cpu, csr, value | zimm);
			cpu->r[rd] = value;
			return;
		case 7:
			// CSRRCI: чтение-сброс битов управляющего регистра (zimm вместо rs1)
			value = rd == 0 ? 0 : csr_read(cpu, csr);
			csr_write(cpu, csr, value & ~zimm);
			cpu->r[rd] = value;
			return;
	}

	trap(cpu, EX_INSTR_ILLEGAL, cpu->instr_pc);
}

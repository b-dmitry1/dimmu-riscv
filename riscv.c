#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "riscv.h"
#include "decode.h"
#include "sbi.h"
#include "platform.h"

// Чтение кода команды
int fetch(riscv_t* cpu, uint32_t* instr)
{
	ui addr;
	ui* phys;

	// Запомнить в instr_pc адрес текущей инструкции на случай исключения
	cpu->instr_pc = cpu->pc;

	// Преобразовать виртуальный адрес инструкции в физический
	// и проверить разрешение на выполнение (X) у страницы памяти
	if (!virt2phys(cpu, &addr, cpu->pc, MMU_X, MMU_ACCESSED, EX_INSTR_ACCESS, EX_INSTR_PAGE_FAULT))
		return 0;

	addr -= RAM_START;
	// Проверить, не выходит ли адрес за пределы физической памяти
	if (addr >= RAM_SIZE)
	{
		trap(cpu, EX_INSTR_ACCESS, cpu->pc);
		return 0;
	}

	// Получить код инструкции
	phys = (ui*)(&cpu->ram[addr]);

	*instr = (uint32_t)*phys;

	// и передвинуть PC на начало следующей инструкции (+16 или +32 бит)
	if ((*instr & 0x03) == 0x03)
	{
		cpu->pc += 4;
	}
	else
	{
		cpu->pc += 2;
		*instr &= 0xFFFF;
	}

	return 1;
}

// Выполнить 1 инструкцию
void do_step(riscv_t* cpu)
{
	uint32_t instr;
	int rd, rs1, rs2, shamt;
	int func3, func7;
	si imm;
	ui addr;
	si value;
	int32_t a32, b32, res32;
	uint32_t u32;
	uint64_t u64;
	int64_t i64;

	// Получить код инструкции
	if (!fetch(cpu, &instr))
		return;

	// Если это 16-битная инструкция, то выполнить в step_compressed
	if ((instr & 3) != 3)
	{
		do_step_compressed(cpu, (uint16_t)instr);
		return;
	}

	// Декодировать поля инструкции
	rd = bits(instr, 11, 7);
	rs1 = bits(instr, 19, 15);
	rs2 = bits(instr, 24, 20);
	shamt = bits(instr, 25, 20);
	func3 = bits(instr, 14, 12);
	func7 = bits(instr, 31, 25);

	// Обнулить значение r0 (hardwired to zero)
	cpu->r[0] = 0;
	
	// Декодировать код команды
	switch (instr & 0x7F)
	{
		case 0x03:
			addr = cpu->r[rs1] + I_imm(instr);
			switch (func3)
			{
				case 0: // LB
					if (!read8(cpu, addr, &value))
						return;
					cpu->r[rd] = (int8_t)value;
					return;
				case 1: // LH
					if (!read16(cpu, addr, &value))
						return;
					cpu->r[rd] = (int16_t)value;
					return;
				case 2: // LW
					if (!read32(cpu, addr, &value))
						return;
					cpu->r[rd] = (int32_t)value;
					return;
				case 3: // LD
					if (!read64(cpu, addr, &value))
						return;
					cpu->r[rd] = value;
					return;
				case 4: // LBU
					if (!read8(cpu, addr, &value))
						return;
					cpu->r[rd] = ((uint8_t)value) & 0xFF;
					return;
				case 5: // LHU
					if (!read16(cpu, addr, &value))
						return;
					cpu->r[rd] = ((uint16_t)value) & 0xFFFF;
					return;
				case 6: // LWU
					if (!read32(cpu, addr, &value))
						return;
					cpu->r[rd] = ((uint32_t)value) & 0xFFFFFFFFu;
					return;
			}
			break;
		case 0x0f:
			// FENCE
			return;
		case 0x13:
			imm = I_imm(instr);
			switch (func3)
			{
				case 0: // ADDI
					cpu->r[rd] = cpu->r[rs1] + imm;
					return;
				case 1: // SLLI
					cpu->r[rd] = cpu->r[rs1] << shamt;
					return;
				case 2: // SLTI
					cpu->r[rd] = cpu->r[rs1] < imm ? 1 : 0;
					return;
				case 3: // SLTIU
					cpu->r[rd] = ((ui)cpu->r[rs1]) < ((ui)imm) ? 1 : 0;
					return;
				case 4: // XORI
					cpu->r[rd] = cpu->r[rs1] ^ imm;
					return;
				case 5:
					if (func7 & 0x20)
					{
						cpu->r[rd] = cpu->r[rs1] >> shamt; // SRAI
					}
					else
					{
						cpu->r[rd] = ((ui)cpu->r[rs1]) >> shamt; // SRLI
					}
					return;
				case 6: // ORI
					cpu->r[rd] = cpu->r[rs1] | imm;
					return;
				case 7: // ANDI
					cpu->r[rd] = cpu->r[rs1] & imm;
					return;
			}
			break;
		case 0x17:
			// AUIPC
			cpu->r[rd] = cpu->instr_pc + U_imm(instr);
			return;
		case 0x1B:
			imm = I_imm(instr);
			a32 = (int32_t)cpu->r[rs1];
			u32 = (uint32_t)cpu->r[rs1];
			switch (func3)
			{
				case 0: // ADDIW
					b32 = (int32_t)imm;
					a32 += b32;
					cpu->r[rd] = a32;
					return;
				case 1: // SLLIW
					a32 <<= shamt & 0x1F;
					cpu->r[rd] = a32;
					return;
				case 5:
					if (func7 & 0x20)
					{
						a32 >>= shamt & 0x1F;
						cpu->r[rd] = a32; // SRAIW
					}
					else
					{
						u32 >>= shamt & 0x1F;
						cpu->r[rd] = u32; // SRLIW
					}
					return;
			}
			break;
		case 0x23:
			// S-type
			addr = cpu->r[rs1] + S_imm(instr);
			switch (func3)
			{
				case 0:
					// SB
					write8(cpu, addr, (uint8_t)cpu->r[rs2]);
					return;
				case 1:
					// SH
					write16(cpu, addr, (uint16_t)cpu->r[rs2]);
					return;
				case 2:
					// SW
					write32(cpu, addr, (uint32_t)cpu->r[rs2]);
					return;
				case 3:
					// SD
					write64(cpu, addr, cpu->r[rs2]);
					return;
			}
			break;
		case 0x2F:
			// Атомарные инструкции
			do_atomic(cpu, instr, rs1, rs2, rd, func3, func7);
			return;
		case 0x33:
			if (func7 & 0x01)
			{
				do_muldiv(cpu, instr, rs1, rs2, rd, func3, func7);
				return;
			}
			else
			{
				switch (func3)
				{
					case 0:
						if (func7 & 0x20)
						{
							cpu->r[rd] = cpu->r[rs1] - cpu->r[rs2]; // SUB
						}
						else
						{
							cpu->r[rd] = cpu->r[rs1] + cpu->r[rs2]; // ADD
						}
						return;
					case 1: // SLL
						cpu->r[rd] = cpu->r[rs1] << (cpu->r[rs2] & 0x3F);
						return;
					case 2: // SLT
						cpu->r[rd] = cpu->r[rs1] < cpu->r[rs2] ? 1 : 0;
						return;
					case 3: // SLTU
						cpu->r[rd] = ((ui)cpu->r[rs1]) < ((ui)cpu->r[rs2]) ? 1 : 0;
						return;
					case 4: // XOR
						cpu->r[rd] = cpu->r[rs1] ^ cpu->r[rs2];
						return;
					case 5:
						if (func7 & 0x20)
						{
							cpu->r[rd] = cpu->r[rs1] >> (cpu->r[rs2] & 0x3F); // SRA
						}
						else
						{
							cpu->r[rd] = ((ui)cpu->r[rs1]) >> (cpu->r[rs2] & 0x3F); // SRL
						}
						return;
					case 6: // OR
						cpu->r[rd] = cpu->r[rs1] | cpu->r[rs2];
						return;
					case 7: // AND
						cpu->r[rd] = cpu->r[rs1] & cpu->r[rs2];
						return;
				}
			}
			break;
		case 0x37: // LUI
			cpu->r[rd] = U_imm(instr);
			return;
		case 0x3B:
			a32 = (int32_t)cpu->r[rs1];
			b32 = (int32_t)cpu->r[rs2];
			if (func7 & 0x01)
			{
				do_muldiv32(cpu, instr, rs1, rs2, rd, func3, func7);
				return;
			}
			else
			{
				switch (func3)
				{
					case 0:
						if (func7 & 0x20)
						{
							res32 = a32 - b32;
							cpu->r[rd] = res32; // SUBW
						}
						else
						{
							res32 = a32 + b32;
							cpu->r[rd] = res32; // ADDW
						}
						return;
					case 1: // SLLW
						res32 = a32 << (b32 & 0x1F);
						cpu->r[rd] = res32;
						return;
					case 5:
						if (func7 & 0x20)
						{
							res32 = a32 >> (b32 & 0x1F);
							cpu->r[rd] = res32; // SRAW
						}
						else
						{
							u32 = ((uint32_t)a32) >> (b32 & 0x1F);
							cpu->r[rd] = u32; // SRLW
						}
						return;
				}
			}
			break;
		case 0x63:
			addr = cpu->instr_pc + B_imm(instr);
			switch (func3)
			{
				case 0:
					// BEQ
					if (cpu->r[rs1] == cpu->r[rs2])
						cpu->pc = addr;
					return;
				case 1:
					// BNE
					if (cpu->r[rs1] != cpu->r[rs2])
						cpu->pc = addr;
					return;
				case 4:
					// BLT
					if (cpu->r[rs1] < cpu->r[rs2])
						cpu->pc = addr;
					return;
				case 5:
					// BGE
					if (cpu->r[rs1] >= cpu->r[rs2])
						cpu->pc = addr;
					return;
				case 6:
					// BLTU
					if (((ui)cpu->r[rs1]) < ((ui)cpu->r[rs2]))
						cpu->pc = addr;
					return;
				case 7:
					// BGEU
					if (((ui)cpu->r[rs1]) >= ((ui)cpu->r[rs2]))
						cpu->pc = addr;
					return;
			}
			break;
		case 0x67:
			// JALR
			addr = cpu->r[rs1] + I_imm(instr);
			cpu->r[rd] = cpu->pc;
			cpu->pc = addr;
			return;
		case 0x6F:
			// JAL
			cpu->r[rd] = cpu->pc;
			cpu->pc = cpu->instr_pc + J_imm(instr);
			return;
		case 0x73:
			do_priv(cpu, instr, rs1, rs2, rd, func3, func7);
			return;
	}
	trap(cpu, EX_INSTR_ILLEGAL, cpu->instr_pc);
}

// Выполнять код 1 микросекунду
void step1us(riscv_t* cpu)
{
	int i, t = 0;

	if (cpu->wfi)
		t = sleep1ms();
	riscv_timer_tick(cpu, t * 1000 + 1);

	plic_update(cpu);

	for (i = 0; i < INSTR_IN_1US && !cpu->wfi; i++)
		do_step(cpu);
}

// Сброс процессора
void reset(riscv_t* cpu)
{
	int i;

	cpu->s_mode = 1;
	cpu->mmu_on = 0;
	cpu->wfi = 0;

	cpu->mtime = 0;
	cpu->mtimecmp = -1;

	for (i = 0; i < 32; i++)
		cpu->r[i] = 0;
}

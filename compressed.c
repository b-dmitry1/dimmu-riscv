#include "riscv.h"
#include "decode.h"

static int32_t signext(int32_t value, int sign, int bits)
{
	if (sign)
		value |= (0xFFFFFFFFFFFFFFFFllu << bits);
	return value;
}

ui get_reg(riscv_t* cpu, int reg)
{
	return cpu->r[reg];
}

si get_regi(riscv_t* cpu, int reg)
{
	return cpu->r[reg];
}

void set_reg(riscv_t* cpu, int reg, si value)
{
	if (reg > 0)
		cpu->r[reg] = value;
}

static void quadrant0(riscv_t* cpu, uint32_t op)
{
	int rs1, rs2, rd, uimm;
	ui addr;
	si value;

	rs2 = rd = bits(op, 4, 2) + 8;
	rs1 = bits(op, 9, 7) + 8;
	switch (bits(op, 15, 13))
	{
		case 0: // ADDI4SPN
			uimm = bit(op, 5) << 3;
			uimm |= bit(op, 6) << 2;
			uimm |= bits(op, 10, 7) << 6;
			uimm |= bits(op, 12, 11) << 4;
			cpu->r[rd] = cpu->r[2] + uimm;
			return;
		case 2: // LW
			uimm = bit(op, 5) << 6;
			uimm |= bit(op, 6) << 2;
			uimm |= bits(op, 12, 10) << 3;
			addr = cpu->r[rs1] + uimm;
			if (read32(cpu, addr, &value))
				cpu->r[rd] = (int32_t)value;
			return;
		case 3: // LD
			uimm = bits(op, 6, 5) << 6;
			uimm |= bits(op, 12, 10) << 3;
			addr = cpu->r[rs1] + uimm;
			if (read64(cpu, addr, &value))
				cpu->r[rd] = value;
			return;
		case 6: // SW
			uimm = bit(op, 5) << 6;
			uimm |= bit(op, 6) << 2;
			uimm |= bits(op, 12, 10) << 3;
			addr = cpu->r[rs1] + uimm;
			write32(cpu, addr, (si)cpu->r[rs2]);
			return;
		case 7: // SD
			uimm = bits(op, 6, 5) << 6;
			uimm |= bits(op, 12, 10) << 3;
			addr = cpu->r[rs1] + uimm;
			write64(cpu, addr, cpu->r[rs2]);
			return;
	}

	trap(cpu, EX_INSTR_ILLEGAL, cpu->instr_pc);
}

static void quadrant1(riscv_t* cpu, uint32_t op)
{
	unsigned int rs1, rs2, rd;
	int32_t uimm;
	int32_t imm;
	int32_t a32, b32, res32;

	switch (bits(op, 15, 13))
	{
		case 0: // ADDI
			imm = bits(op, 6, 2);
			imm = signext(imm, bit(op, 12), 5);
			rd = bits(op, 11, 7);
			cpu->r[rd] += imm;
			return;
		case 1:
#if XLEN == 32
			// JAL
			imm = bit(op, 2) << 5;
			imm |= bits(op, 5, 3) << 1;
			imm |= bit(op, 6) << 7;
			imm |= bit(op, 7) << 6;
			imm |= bit(op, 8) << 10;
			imm |= bits(op, 10, 9) << 8;
			imm |= bit(op, 11) << 4;
			imm = signext(imm, bit(op, 12), 11);
			cpu->r[1] = cpu->pc;
			cpu->pc += imm - 2;
#else
			// ADDIW
			imm = bits(op, 6, 2);
			imm = signext(imm, bit(op, 12), 5);
			rd = bits(op, 11, 7);
			a32 = (int32_t)get_regi(cpu, rd);
			b32 = (int32_t)imm;
			res32 = a32 + b32;
			set_reg(cpu, rd, res32);
#endif
			return;
		case 2: // LI
			imm = bits(op, 6, 2);
			imm = signext(imm, bit(op, 12), 5);
			rd = bits(op, 11, 7);
			set_reg(cpu, rd, imm);
			return;
		case 3: // LUI / ADDI16SP
			rd = bits(op, 11, 7);
			if (rd == 2)
			{
				// ADDI16SP
				imm = bit(op, 2) << 5;
				imm |= bits(op, 4, 3) << 7;
				imm |= bit(op, 5) << 6;
				imm |= bit(op, 6) << 4;
				imm = signext(imm, bit(op, 12), 9);
				set_reg(cpu, 2, get_reg(cpu, 2) + imm);
			}
			else
			{
				// LUI
				imm = bits(op, 6, 2) << 12;
				imm = signext(imm, bit(op, 12), 17);
				set_reg(cpu, rd, imm);
			}
			return;
		case 4:
			rs2 = bits(op, 4, 2) + 8;
			rs1 = rd = bits(op, 9, 7) + 8;
			switch (bits(op, 12, 10))
			{
				case 0:
				case 4: // SRLI
					uimm = bits(op, 6, 2);
					uimm |= bit(op, 12) << 5;
					set_reg(cpu, rd, get_reg(cpu, rd) >> uimm);
					return;
				case 1:
				case 5: // SRAI
					uimm = bits(op, 6, 2);
					uimm |= bit(op, 12) << 5;
					set_reg(cpu, rd, get_regi(cpu, rd) >> uimm);
					return;
				case 2:
				case 6: // ANDI
					uimm = bits(op, 6, 2);
					uimm = signext(uimm, bit(op, 12), 5);
					set_reg(cpu, rd, get_reg(cpu, rd) & uimm);
					return;
				case 3:
					switch (bits(op, 6, 5))
					{
						case 0: // SUB
							set_reg(cpu, rd, get_reg(cpu, rs1) - get_reg(cpu, rs2));
							return;
						case 1: // XOR
							set_reg(cpu, rd, get_reg(cpu, rs1) ^ get_reg(cpu, rs2));
							return;
						case 2: // OR
							set_reg(cpu, rd, get_reg(cpu, rs1) | get_reg(cpu, rs2));
							return;
						case 3: // AND
							set_reg(cpu, rd, get_reg(cpu, rs1) & get_reg(cpu, rs2));
							return;
					}
					break;
				case 7:
					switch (bits(op, 6, 5))
					{
						case 0: // SUBW
							a32 = (int32_t)get_regi(cpu, rs1);
							b32 = (int32_t)get_regi(cpu, rs2);
							res32 = a32 - b32;
							set_reg(cpu, rd, res32);
							return;
						case 1: // ADDW
							a32 = (int32_t)get_regi(cpu, rs1);
							b32 = (int32_t)get_regi(cpu, rs2);
							res32 = a32 + b32;
							set_reg(cpu, rd, res32);
							return;
					}
					break;
			}
			break;
		case 5: // J
			imm = bit(op, 2) << 5;
			imm |= bits(op, 5, 3) << 1;
			imm |= bit(op, 6) << 7;
			imm |= bit(op, 7) << 6;
			imm |= bit(op, 8) << 10;
			imm |= bits(op, 10, 9) << 8;
			imm |= bit(op, 11) << 4;
			imm = signext(imm, bit(op, 12), 11);
			cpu->pc += imm - 2;
			return;
		case 6: // BEQZ
			rs1 = bits(op, 9, 7) + 8;
			imm = bit(op, 2) << 5;
			imm |= bits(op, 4, 3) << 1;
			imm |= bits(op, 6, 5) << 6;
			imm |= bits(op, 11, 10) << 3;
			imm = signext(imm, bit(op, 12), 8);
			if (cpu->r[rs1] == 0)
				cpu->pc += imm - 2;
			return;
		case 7: // BNEZ
			rs1 = bits(op, 9, 7) + 8;
			imm = bit(op, 2) << 5;
			imm |= bits(op, 4, 3) << 1;
			imm |= bits(op, 6, 5) << 6;
			imm |= bits(op, 11, 10) << 3;
			imm = signext(imm, bit(op, 12), 8);
			if (cpu->r[rs1] != 0)
				cpu->pc += imm - 2;
			return;
	}

	trap(cpu, EX_INSTR_ILLEGAL, cpu->instr_pc);
}

static void quadrant2(riscv_t* cpu, uint32_t op)
{
	unsigned int rs1, rs2, rd;
	si uimm;
	ui addr;
	si value;

	switch (bits(op, 15, 13))
	{
		case 0: // SLLI
			rd = bits(op, 11, 7);
			uimm = bits(op, 6, 2);
			uimm |= bit(op, 12) << 5;
			set_reg(cpu, rd, get_reg(cpu, rd) << uimm);
			return;
		case 2: // LWSP
			rd = bits(op, 11, 7);
			uimm = bits(op, 3, 2) << 6;
			uimm |= bits(op, 6, 4) << 2;
			uimm |= bit(op, 12) << 5;
			addr = cpu->r[2] + uimm;
			if (read32(cpu, addr, &value))
				set_reg(cpu, rd, value);
			return;
		case 3: // LDSP
			rd = bits(op, 11, 7);
			uimm = bits(op, 4, 2) << 6;
			uimm |= bits(op, 6, 5) << 3;
			uimm |= bit(op, 12) << 5;
			addr = cpu->r[2] + uimm;
			if (read64(cpu, addr, &value))
				set_reg(cpu, rd, value);
			return;
		case 4:
			rs1 = rd = bits(op, 11, 7);
			rs2 = bits(op, 6, 2);
			switch (bit(op, 12))
			{
				case 0:
					if (rs2 == 0)
					{
						cpu->pc = get_reg(cpu, rs1); // JR
					}
					else
					{
						set_reg(cpu, rd, get_reg(cpu, rs2));
					}
					return;
				case 1:
					if (rd == 0 && rs2 == 0)
					{
						// EBREAK
						trap(cpu, EX_BREAKPOINT, 0);
					}
					else if (rs2 == 0)
					{
						set_reg(cpu, 1, cpu->pc);
						cpu->pc = get_reg(cpu, rs1);
					}
					else
					{
						set_reg(cpu, rd, get_reg(cpu, rd) + get_reg(cpu, rs2));
					}
					return;
			}
			break;
		case 6: // SWSP
			rs2 = bits(op, 6, 2);
			uimm = bits(op, 8, 7) << 6;
			uimm |= bits(op, 12, 9) << 2;
			addr = cpu->r[2] + uimm;
			write32(cpu, addr, (int32_t)get_reg(cpu, rs2));
			return;
		case 7: // SDSP
			rs2 = bits(op, 6, 2);
			uimm = bits(op, 9, 7) << 6;
			uimm |= bits(op, 12, 10) << 3;
			addr = cpu->r[2] + uimm;
			write64(cpu, addr, get_regi(cpu, rs2));
			return;
	}

	trap(cpu, EX_INSTR_ILLEGAL, cpu->instr_pc);
}

// 16-битные инструкции
void step_compressed(riscv_t* cpu, uint16_t instr)
{
	// ¬ зависимости от значений битов 0 и 1 кода инструкции,
	// выполнить из соответствующего квадранта.
	// ¬ каждом квадранте по-разному декодируютс€ номера регистров
	switch (instr & 0x03)
	{
		case 0:
			quadrant0(cpu, instr);
			break;
		case 1:
			quadrant1(cpu, instr);
			break;
		case 2:
			quadrant2(cpu, instr);
			break;
	}
}

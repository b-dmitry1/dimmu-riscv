#include "riscv.h"
#include "decode.h"

// Расширение "M" - аппаратное умножение и деление

// Умножение и деление
void do_muldiv(riscv_t* cpu, uint32_t instr, int rs1, int rs2, int rd, int func3, int func7)
{
	switch (func3)
	{
		case 0: // MUL - умножение со знаком
			cpu->r[rd] = cpu->r[rs1] * cpu->r[rs2];
			return;
		case 1: // MULH - умножение со знаком, взятие старшей части результата
			cpu->r[rd] = (cpu->r[rs1] * ((int64_t)cpu->r[rs2])) >> XLEN;
			return;
		case 2: // MULHSU - умножение числа со знаком на число без знака, взятие старшей части результата
			cpu->r[rd] = (((int64_t)cpu->r[rs1]) * (uint64_t)cpu->r[rs2]) >> XLEN;
			return;
		case 3: // MULHU - беззнаковое умножение, взятие старшей части результата
#if XLEN == 64
			// Не все компиляторы умеют умножать uint64_t на uint64_t
			// Здесь пока вернём ноль, для Linux64 этого будет достаточно
			cpu->r[rd] = 0; 
#else
			cpu->r[rd] = (((uint64_t)(ui)cpu->r[rs1]) * (uint64_t)(ui)cpu->r[rs2]) >> XLEN;
#endif
			return;
		case 4: // DIV - деление со знаком
			if (cpu->r[rs2] == 0)
				cpu->r[rd] = -1;
			else
				cpu->r[rd] = cpu->r[rs1] / cpu->r[rs2];
			return;
		case 5: // DIVU - деление без знака
			if (cpu->r[rs2] == 0)
				cpu->r[rd] = -1;
			else
				cpu->r[rd] = ((ui)cpu->r[rs1]) / ((ui)cpu->r[rs2]);
			return;
		case 6: // REM - вычисление остатка от деления со знаком
			if (cpu->r[rs2] == 0)
				cpu->r[rd] = cpu->r[rs1];
			else
				cpu->r[rd] = cpu->r[rs1] % cpu->r[rs2];
			return;
		case 7: // REMU - вычисление остатка от деления без знака
			if (cpu->r[rs2] == 0)
				cpu->r[rd] = cpu->r[rs1];
			else
				cpu->r[rd] = ((ui)cpu->r[rs1]) % ((ui)cpu->r[rs2]);
			return;
	}
}

// Умножение и деление 32-битных чисел на 64-битном процессоре
void do_muldiv32(riscv_t* cpu, uint32_t instr, int rs1, int rs2, int rd, int func3, int func7)
{
	int32_t a32, b32, res32;

	a32 = (int32_t)cpu->r[rs1];
	b32 = (int32_t)cpu->r[rs2];

	switch (func3)
	{
		case 0: // MULW - умножение со знаком
			res32 = a32 * b32;
			cpu->r[rd] = res32;
			return;
		case 4: // DIVW - деление со знаком
			if (b32 == 0)
				res32 = -1;
			else
				res32 = a32 / b32;
			cpu->r[rd] = res32;
			return;
		case 5: // DIVUW - деление без знака
			if (b32 == 0)
				cpu->r[rd] = -1;
			else
				cpu->r[rd] = ((uint32_t)a32) / ((uint32_t)b32);
			return;
		case 6: // REMW - вычисление остатка от деления со знаком
			res32 = a32 % b32;
			cpu->r[rd] = res32;
			return;
		case 7: // REMUW - вычисление остатка от деления без знака
			cpu->r[rd] = ((uint32_t)a32) % ((uint32_t)b32);
			return;
	}

	trap(cpu, EX_INSTR_ILLEGAL, cpu->instr_pc);
}

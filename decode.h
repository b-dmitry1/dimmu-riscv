#pragma once

#include <stdint.h>

static int32_t bit(uint32_t value, int number)
{
	return (value >> number) & 1;
}

static int32_t bits(uint32_t value, int high, int low)
{
	value >>= low;
	value &= (1ll << (high - low + 1)) - 1llu;
	return value;
}

static int32_t I_imm(int32_t op)
{
	int32_t res = op >> 20;
	return res;
}

static uint32_t I_csr(uint32_t op)
{
	return op >> 20;
}

static int32_t S_imm(int32_t op)
{
	int32_t res = op >> 25;
	res <<= 5;
	res |= bits(op, 11, 7);
	return res;
}

static int32_t B_imm(int32_t op)
{
	int32_t res = op >> 31;
	res <<= 12;
	res |= bits(op, 30, 25) << 5;
	res |= bits(op, 11, 8) << 1;
	res |= bit(op, 7) << 11;
	return res;
}

static int32_t U_imm(int32_t op)
{
	int32_t res = op & 0xFFFFF000;
	return res;
}

static int32_t J_imm(int32_t op)
{
	int32_t res = op >> 31;
	res <<= 20;
	res |= bits(op, 30, 21) << 1;
	res |= bit(op, 20) << 11;
	res |= bits(op, 19, 12) << 12;
	return res;
}

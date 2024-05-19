#include <stdio.h>
#include "riscv.h"

// ������ 1 �����
int read8(riscv_t* cpu, ui addr, si* result)
{
	ui phys;

	// ������������� ����������� ����� � ����������
	if (!virt2phys(cpu, &phys, addr, MMU_R, MMU_ACCESSED, EX_LOAD_ACCESS, EX_LOAD_PAGE_FAULT))
		return 0;

	// ���������, �� ������� �� �� ������� ���
	if ((phys - RAM_START) < RAM_SIZE)
	{
		// �� ��, ������� ���������
		*result = *((int8_t*)&cpu->ram[phys - RAM_START]);
		return 1;
	}

	// ����� �� ������� ���. ���� ����� �������� ��������� � ������ ����������� �� ����

	// �� ���� �� ��������� �� ���������� ������, ������� ������ ������
	*result = 0;
	return 0;
}

int read16(riscv_t* cpu, ui addr, si* result)
{
	ui phys;

	if (!virt2phys(cpu, &phys, addr, MMU_R, MMU_ACCESSED, EX_LOAD_ACCESS, EX_LOAD_PAGE_FAULT))
		return 0;

	if ((phys - RAM_START) < RAM_SIZE)
	{
		*result = *((int16_t*)&cpu->ram[phys - RAM_START]);
		return 1;
	}

	*result = 0;
	return 0;
}

int read32(riscv_t* cpu, ui addr, si* result)
{
	ui phys;

	if (!virt2phys(cpu, &phys, addr, MMU_R, MMU_ACCESSED, EX_LOAD_ACCESS, EX_LOAD_PAGE_FAULT))
		return 0;

	if ((phys - RAM_START) < RAM_SIZE)
	{
		*result = *((int32_t*)&cpu->ram[phys - RAM_START]);
		return 1;
	}

	*result = 0;
	return 0;
}

int read64(riscv_t* cpu, ui addr, si* result)
{
	ui phys;

	if (!virt2phys(cpu, &phys, addr, MMU_R, MMU_ACCESSED, EX_LOAD_ACCESS, EX_LOAD_PAGE_FAULT))
		return 0;

	if ((phys - RAM_START) < RAM_SIZE)
	{
		*result = *((int64_t*)&cpu->ram[phys - RAM_START]);
		return 1;
	}

	*result = 0;
	return 0;
}

int write8(riscv_t* cpu, ui addr, int8_t value)
{
	ui phys;

	if (!virt2phys(cpu, &phys, addr, MMU_W, MMU_ACCESSED | MMU_DIRTY, EX_STORE_ACCESS, EX_STORE_PAGE_FAULT))
		return 0;

	if ((phys - RAM_START) < RAM_SIZE)
	{
		*((int8_t*)&cpu->ram[phys - RAM_START]) = value;
		return 1;
	}

	return 0;
}

int write16(riscv_t* cpu, ui addr, int16_t value)
{
	ui phys;

	if (!virt2phys(cpu, &phys, addr, MMU_W, MMU_ACCESSED | MMU_DIRTY, EX_STORE_ACCESS, EX_STORE_PAGE_FAULT))
		return 0;

	if ((phys - RAM_START) < RAM_SIZE)
	{
		*((int16_t*)&cpu->ram[phys - RAM_START]) = value;
		return 1;
	}

	return 0;
}

int write32(riscv_t* cpu, ui addr, int32_t value)
{
	ui phys;

	if (!virt2phys(cpu, &phys, addr, MMU_W, MMU_ACCESSED | MMU_DIRTY, EX_STORE_ACCESS, EX_STORE_PAGE_FAULT))
		return 0;

	if ((phys - RAM_START) < RAM_SIZE)
	{
		*((int32_t*)&cpu->ram[phys - RAM_START]) = value;
		return 1;
	}

	return 0;
}

int write64(riscv_t* cpu, ui addr, int64_t value)
{
	ui phys;

	if (!virt2phys(cpu, &phys, addr, MMU_W, MMU_ACCESSED | MMU_DIRTY, EX_STORE_ACCESS, EX_STORE_PAGE_FAULT))
		return 0;

	if ((phys - RAM_START) < RAM_SIZE)
	{
		*((int64_t*)&cpu->ram[phys - RAM_START]) = value;
		return 1;
	}

	return 0;
}

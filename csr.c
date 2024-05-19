#include "riscv.h"

#define CSR_SSTATUS			0x100
#define CSR_SIE				0x104
#define CSR_STVEC			0x105
#define CSR_SSCRATCH		0x140
#define CSR_SEPC			0x141
#define CSR_SCAUSE			0x142
#define CSR_STVAL			0x143
#define CSR_SIP				0x144
#define CSR_SATP			0x180

#define CSR_MSTATUS			0x300
#define CSR_MIE				0x304
#define CSR_MTVEC			0x305
#define CSR_MSCRATCH		0x340
#define CSR_MEPC			0x341
#define CSR_MCAUSE			0x342
#define CSR_MTVAL			0x343
#define CSR_MIP				0x344

#define CSR_TIMECMP			0x800
#define CSR_TIMECMPH		0x801
#define CSR_TIME			0xC01
#define CSR_TIMEH			0xC81

#include <stdio.h>

// Чтение управляющих регистров
ui csr_read(riscv_t* cpu, uint32_t number)
{
	// printf("Read CSR: %x\n", number);
	switch (number)
	{
	case CSR_SSTATUS: return cpu->sstatus;
		case CSR_SIE: return cpu->sie;
		case CSR_STVEC: return cpu->stvec;
		case CSR_SSCRATCH: return cpu->sscratch;
		case CSR_SEPC: return cpu->sepc;
		case CSR_SCAUSE: return cpu->scause;
		case CSR_STVAL: return cpu->stval;
		case CSR_SIP: return cpu->sip;
		case CSR_SATP: return cpu->satp;

#if XLEN == 64
		case CSR_TIME: return cpu->mtime;
		case CSR_TIMECMP: return cpu->mtimecmp;
#else
		case CSR_TIME: return cpu->mtime & 0xFFFFFFFF;
		case CSR_TIMEH: return cpu->mtime >> 32;
		case CSR_TIMECMP: return cpu->mtimecmp & 0xFFFFFFFF;
		case CSR_TIMECMPH: return cpu->mtimecmp >> 32;
#endif
	}
	return 0;
}

// Запись в управляющие регистры
void csr_write(riscv_t* cpu, uint32_t number, ui value)
{
	// printf("Write CSR: %x, %llx\n", number, value);

	switch (number)
	{
		case CSR_SSTATUS: cpu->sstatus = value; break;
		case CSR_SIE: cpu->sie = value; break;
		case CSR_STVEC: cpu->stvec = value; break;
		case CSR_SSCRATCH: cpu->sscratch = value; break;
		case CSR_SEPC: cpu->sepc = value; break;
		case CSR_SCAUSE: cpu->scause = value; break;
		case CSR_STVAL: cpu->stval = value; break;
		case CSR_SIP: cpu->sip = value; break;
		case CSR_SATP: // Установка режима MMU и адреса каталога страниц
			cpu->satp = set_atp(cpu, value);
			break;

#if XLEN == 64
		case CSR_TIME: set_mtime64(cpu, value); break;
		case CSR_TIMECMP: set_mtimecmp64(cpu, value); break;
#else
		case CSR_TIME: set_mtime(cpu, cpu->mtime >> 32, value); break;
		case CSR_TIMEH: set_mtime(cpu, value, cpu->mtime & 0xFFFFFFFF); break;
		case CSR_TIMECMP: set_mtimecmp(cpu, cpu->mtimecmp >> 32, value); break;
		case CSR_TIMECMPH: set_mtimecmp(cpu, value, cpu->mtimecmp & 0xFFFFFFFF); break;
#endif
	}
}

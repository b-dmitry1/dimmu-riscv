#pragma once

#include <stdint.h>

#include "config.h"

// Код режима работы MMU и константы, зависящие от разрядности
#if XLEN == 64

// Регистры 64-битные
typedef uint64_t ui;
typedef int64_t si;
// Биты включения MMU
#define SATP_MODE_ENABLE			0x8000000000000000llu
#define SATP_MODE_MASK				0xF000000000000000llu
// Количество уровней в каталогах страниц
#define MMU_LEVELS					3
// Количество битов, определяющих номер страницы в каталоге
#define MMU_LEVEL_BITS				9
// Битовая маска номера страницы в каталоге
#define MMU_VPN_MASK				0x1FFu
// Битовая маска для определения размера огромной страницы (если только 1 уровень)
#define MMU_PAGE_MASK				0x3FFFFFFFllu
// Флаг, отличающий прерывание от исключительной ситуации
#define CAUSE_IRQ					0x8000000000000000llu

#else

// Регистры 32-битные
typedef uint32_t ui;
typedef int32_t si;
#define SATP_MODE_ENABLE			0x80000000u
#define SATP_MODE_MASK				0x80000000u
#define MMU_LEVELS					2
#define MMU_LEVEL_BITS				10
#define MMU_VPN_MASK				0x3FFu
#define MMU_PAGE_MASK				0x3FFFFFu
#define CAUSE_IRQ					0x80000000u

#endif

// Коды исключительных ситуаций
#define EX_INSTR_MISALIGNED			0
#define EX_INSTR_ACCESS				1
#define EX_INSTR_ILLEGAL			2
#define EX_BREAKPOINT				3
#define EX_LOAD_MISALIGNED			4
#define EX_LOAD_ACCESS				5
#define EX_STORE_MISALIGNED			6
#define EX_STORE_ACCESS				7
#define EX_ECALL_U					8
#define EX_ECALL_S					9
#define EX_ECALL_M					11
#define EX_INSTR_PAGE_FAULT			12
#define EX_LOAD_PAGE_FAULT			13
#define EX_STORE_PAGE_FAULT			15

// Коды прерываний
#define INT_U_SOFT					(CAUSE_IRQ | 0x0)
#define INT_S_SOFT					(CAUSE_IRQ | 0x1)
#define INT_M_SOFT					(CAUSE_IRQ | 0x3)
#define INT_U_TIMER					(CAUSE_IRQ | 0x4)
#define INT_S_TIMER					(CAUSE_IRQ | 0x5)
#define INT_M_TIMER					(CAUSE_IRQ | 0x7)
#define INT_U_EXT					(CAUSE_IRQ | 0x8)
#define INT_S_EXT					(CAUSE_IRQ | 0x9)
#define INT_M_EXT					(CAUSE_IRQ | 0xb)

// Битовые поля в каталогах страниц виртуальной памяти
// V (valid) - запись существует
#define MMU_V						0x0001
// R (read) - разрешение чтения из страницы памяти
#define MMU_R						0x0002
// W (write) - разрешение записи в страницу памяти
#define MMU_W						0x0004
// X (execute) - разрешение выполнения кода из страницы памяти
#define MMU_X						0x0008
// USER - разрешение доступа из пользовательского режима
#define MMU_USER					0x0010
// GLOBAL - флаг разделяемой страницы, она существует во всех адресных пространствах
#define MMU_GLOBAL					0x0020
// ACCESSED - индикатор, показывающий, что был доступ к содержимому страницы
#define MMU_ACCESSED				0x0040
// DIRTY - индикатор, показывающий, что было изменение содержимого страницы
#define MMU_DIRTY					0x0080

// Флаг разрешения прерывания таймера
#define MIE_MTIE					(1 << 5)
// Флаги регистра состояния
#define SSTATUS_SIE					(1 << 1)
#define SSTATUS_SPIE				(1 << 5)
#define SSTATUS_SPP					(1 << 8)
#define SSTATUS_SUM					(1 << 18)

// Ядро RISC-V
typedef struct
{
	// Регистры общего назначения
	si r[32];
	// Счётчик команд и адрес текущей команды
	ui pc;
	ui instr_pc;
	// Резерв адреса для атомарных операций
	ui res_addr;
	// Режим работы (0 - пользователь, 1 - супервизор)
	int s_mode;
	// Флаг сна до появления прерывания
	int wfi;

	// Указатель на начало блока физической памяти
	uint8_t* ram;

	// Управляющие регистры
	ui sstatus;  // Регистр состояния
	ui sie;      // Регистр разрешения прерывания
	ui stvec;    // Регистр адреса обработчика прерываний
	ui sscratch; // Регистр контекста
	ui sepc;     // Адрес возврата из прерывания
	ui scause;   // Код исключения/прерывания
	ui stval;    // Значение (обычно, адрес) для обработчика исключения
	ui sip;      // Регистр флагов прерываний
	ui satp;     // Регистр MMU (режим + адрес каталога)

	// Системный таймер
	int64_t mtime;
	int64_t mtimecmp;

	// Указатель на главный каталог страниц виртуальной памяти
	ui* atp;
	// Флаг включения MMU
	int mmu_on;
} riscv_t;

// Функции MMU
ui set_atp(riscv_t* cpu, ui value);
int virt2phys(riscv_t* cpu, ui* res, ui virt, ui test, ui set, ui cause1, ui cause2);

// Функции исключений/прерываний
void trap(riscv_t* cpu, ui cause, ui value);
void sret(riscv_t* cpu);
void plic_update(riscv_t* cpu);

// Функции таймера
void set_mtime(riscv_t* cpu, uint32_t high, uint32_t low);
void set_mtimecmp(riscv_t* cpu, uint32_t high, uint32_t low);
void set_mtime64(riscv_t* cpu, uint64_t value);
void set_mtimecmp64(riscv_t* cpu, uint64_t value);
void riscv_timer_tick(riscv_t* cpu, int useconds);

// Функции чтения/записи системной шины
int read8(riscv_t* cpu, ui addr, si* result);
int read16(riscv_t* cpu, ui addr, si* result);
int read32(riscv_t* cpu, ui addr, si* result);
int read64(riscv_t* cpu, ui addr, si* result);
int write8(riscv_t* cpu, ui addr, int8_t value);
int write16(riscv_t* cpu, ui addr, int16_t value);
int write32(riscv_t* cpu, ui addr, int32_t value);
int write64(riscv_t* cpu, ui addr, int64_t value);

// Функции чтения/записи управляющих регистров
ui csr_read(riscv_t* cpu, uint32_t number);
void csr_write(riscv_t* cpu, uint32_t number, ui value);

// Расширение "A" - Атомарные инструкции
void do_atomic(riscv_t* cpu, uint32_t instr, int rs1, int rs2, int rd, int func3, int func7);
// Расширение "M" - Аппаратное умножение и деление
void do_muldiv(riscv_t* cpu, uint32_t instr, int rs1, int rs2, int rd, int func3, int func7);
void do_muldiv32(riscv_t* cpu, uint32_t instr, int rs1, int rs2, int rd, int func3, int func7);
// Привелегированные инструкции
void do_priv(riscv_t* cpu, uint32_t instr, int rs1, int rs2, int rd, int func3, int func7);

// Функции управления процессором
void do_step(riscv_t* cpu);
void do_step_compressed(riscv_t* cpu, uint16_t instr);
void step1us(riscv_t* cpu);
void reset(riscv_t* cpu);

#include "riscv.h"

// Обновление флага прерывания таймера
// Вызывается после каждой записи в регистры time или timecmp
static void update_timer_irq(riscv_t* cpu)
{
	cpu->sip &= ~MIE_MTIE;
	if (cpu->mtime > cpu->mtimecmp)
		cpu->sip |= MIE_MTIE;
}

void set_mtime(riscv_t* cpu, uint32_t high, uint32_t low)
{
	cpu->mtime = low | (((int64_t)high) << 32);
	update_timer_irq(cpu);
}

void set_mtimecmp(riscv_t* cpu, uint32_t high, uint32_t low)
{
	cpu->mtimecmp = low | (((int64_t)high) << 32);
	update_timer_irq(cpu);
}

void set_mtime64(riscv_t* cpu, uint64_t value)
{
	cpu->mtime = value;
	update_timer_irq(cpu);
}

void set_mtimecmp64(riscv_t* cpu, uint64_t value)
{
	cpu->mtimecmp = value;
	update_timer_irq(cpu);
}

// Перевод таймера вперёд на указанное кол-во микросекунд
void riscv_timer_tick(riscv_t* cpu, int useconds)
{
	set_mtime64(cpu, cpu->mtime + useconds);
}

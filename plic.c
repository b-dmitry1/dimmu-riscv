#include "riscv.h"

// Обновление состояния контроллера прерываний
void plic_update(riscv_t* cpu)
{
	// Реакция на прерывание таймера
	if ((cpu->sip & cpu->sie & MIE_MTIE) && ((cpu->sstatus & SSTATUS_SIE) || cpu->wfi))
	{
		cpu->wfi = 0;
		trap(cpu, INT_S_TIMER, 0);
	}
}

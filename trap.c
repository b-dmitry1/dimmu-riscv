#include "riscv.h"

// Вызов функции прерывания или обработчика исключительной ситуации
void trap(riscv_t* cpu, ui cause, ui value)
{
	// Сохранить код исключения
	cpu->scause = cause;
	cpu->stval = value;

	// Сохранить в SPIE состояние флага разрешения прерываний
	if (cpu->sstatus & SSTATUS_SIE)
		cpu->sstatus |= SSTATUS_SPIE;
	else
		cpu->sstatus &= ~SSTATUS_SPIE;
	// И запретить прерывания
	cpu->sstatus &= ~SSTATUS_SIE;

	// Если исключение вызвано из режима супервизора, то установить SPP
	if (cpu->s_mode)
		cpu->sstatus |= SSTATUS_SPP;
	else
		cpu->sstatus &= ~SSTATUS_SPP;

	// Продолжаем в режиме супервизора
	cpu->s_mode = 1;

	// Если это прерывание, то записать в SEPC адрес следующей инструкции
	// Если это исключение, то записать в SEPC адрес текущей инструкции
	if (cause & CAUSE_IRQ)
		cpu->sepc = cpu->pc;
	else
		cpu->sepc = cpu->instr_pc;

	// Продолжить выполнение по адресу, указанному в регистре STVEC
	cpu->pc = cpu->stvec;

	// Если младший бит STVEC равен 1, то продолжить по адресу
	// STVEC + номер_исключения * 4
	if (cpu->stvec & 1)
	{
		cpu->pc &= ~1;
		cpu->pc += cpu->scause * 4;
	}
}

// Возврат из функции прерывания или из обработчика исключительной ситуации
void sret(riscv_t* cpu)
{
	// Если SPP = 0, то это возврат в пользовательский режим
	// Если SPP = 1, то в супервизор
	cpu->s_mode = cpu->sstatus & SSTATUS_SPP ? 1 : 0;
	// После установки режима SPP сбрасывается
	cpu->sstatus &= ~SSTATUS_SPP;

	// При возврате SPIE копируется в SIE, так восстанавливается
	// флаг прерываний до trap
	if (cpu->sstatus & SSTATUS_SPIE)
		cpu->sstatus |= SSTATUS_SIE;
	else
		cpu->sstatus &= ~SSTATUS_SIE;
	// И SPIE устанавливается в 1
	cpu->sstatus |= SSTATUS_SPIE;

	// Продолжить выполнение программы по сохранённому адресу
	cpu->pc = cpu->sepc;
}

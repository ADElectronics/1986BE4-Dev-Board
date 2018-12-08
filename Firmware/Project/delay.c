#include "delay.h"

volatile uint32_t delay_ms;

// Не документированная возможность для 1986ВЕ4 !
// В реальных проектах полагаться на наличие SysTick НЕ стоит!

void _delay_Init(void)
{
	SysTick_Config((SystemCoreClock /*(HSE_Value * 4)*/ / 1000)-1);
}

void _delay_ms(uint32_t ms)
{
	delay_ms = ms;
	while (delay_ms) {};
}

void SysTick_Handler(void)
{
	if (delay_ms) delay_ms--;
}

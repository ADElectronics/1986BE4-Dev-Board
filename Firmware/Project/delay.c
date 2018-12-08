#include "delay.h"

volatile uint32_t delay_ms;

// �� ����������������� ����������� ��� 1986��4 !
// � �������� �������� ���������� �� ������� SysTick �� �����!

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

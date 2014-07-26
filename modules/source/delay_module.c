/*
 * delay_module.c
 *
 *  Created on: 15.03.2014
 *      Author: Konstantin
 */

#include "delay_module.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_clkpwr.h"


void delay_init(void)
{
	TIM_TIMERCFG_Type Timer_config;

	uint32_t timer_clock = CLKPWR_GetPCLK(CLKPWR_PCLKSEL_TIMER0);

	Timer_config.PrescaleOption = TIM_PRESCALE_TICKVAL;
	Timer_config.PrescaleValue  = timer_clock / 1000000;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &Timer_config);

	TIM_Cmd(LPC_TIM0, ENABLE);
}

inline void delay_us(uint32_t delay)
{
	uint32_t start = LPC_TIM0->TC;

	while ((LPC_TIM0->TC - start) < (delay));
}

inline void delay_ms(uint32_t delay)
{
	uint32_t start = LPC_TIM0->TC;

	while ((LPC_TIM0->TC - start) < (delay * 1000));
}

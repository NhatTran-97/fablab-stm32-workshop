#ifndef BUZZER_H
#define BUZZER_H

#include "stm32l4xx_hal.h"

typedef struct
{
	TIM_HandleTypeDef *htim_buzzer;
	uint32_t channel;
	uint8_t state;
	uint32_t last_time_on;
	
}Buzzer;

void Buzzer_Init(Buzzer *buzzer, TIM_HandleTypeDef *buzzer_htim, uint32_t channel);

static void Buzzer_SetState(Buzzer *buzzer, uint8_t state);

void Buzzer_Toggle(Buzzer *buzzer);

void Buzzer_BeepTick(Buzzer *buzzer, uint16_t period_ms);


#endif
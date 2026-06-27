
#include "buzzer.h" 

#define BUZZER_OFF  0
#define BUZZER_ON   1

#define BUZZER_DUTY_PERCENT   40  

void Buzzer_Init(Buzzer *buzzer, TIM_HandleTypeDef *buzzer_htim, uint32_t channel)
{
	buzzer->htim_buzzer = buzzer_htim;
	buzzer->channel = channel;
	HAL_TIMEx_PWMN_Start(buzzer->htim_buzzer, buzzer->channel );
	
	buzzer->last_time_on = HAL_GetTick();
	buzzer->state = BUZZER_OFF;
	
}

static void Buzzer_SetState(Buzzer *buzzer, uint8_t state)
{
	buzzer->state = state;
	if(state == BUZZER_ON)
	{
			//uint32_t arr = __HAL_TIM_GET_AUTORELOAD(buzzer->htim_buzzer);
			uint32_t arr = buzzer->htim_buzzer ->Instance->ARR;   //could be replace by __HAL_TIM_GET_AUTORELOAD(buzzer->htim_buzzer);
			uint32_t ccr = (arr + 1) * BUZZER_DUTY_PERCENT / 100;
		__HAL_TIM_SET_COMPARE(buzzer->htim_buzzer, buzzer->channel, ccr);
	}
	else
	{
		__HAL_TIM_SET_COMPARE(buzzer->htim_buzzer, buzzer->channel, BUZZER_OFF);
	}
	
}


void Buzzer_Toggle(Buzzer *buzzer)
{
	Buzzer_SetState(buzzer,(buzzer->state == BUZZER_ON) ? BUZZER_OFF : BUZZER_ON);
}


void Buzzer_BeepTick(Buzzer *buzzer, uint16_t period_ms)
{
	uint32_t now = HAL_GetTick();
	if(now - buzzer->last_time_on >= period_ms)
	{
		Buzzer_Toggle(buzzer);
		buzzer->last_time_on = now;
	}
	
}


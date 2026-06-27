#ifndef BATTERY_H
#define BATTERY_H
#include "stm32l4xx_hal.h"


typedef struct
{

	ADC_HandleTypeDef *adc_bat;
	
}BATTERY;


void battery_init(BATTERY *battery, ADC_HandleTypeDef *hadc);

uint16_t get_adc_channel(BATTERY *battery, uint32_t channel);

float battery_get_adc_voltage(BATTERY *battery, uint32_t channel);

float get_battery(BATTERY *battery, uint32_t channel);



#endif
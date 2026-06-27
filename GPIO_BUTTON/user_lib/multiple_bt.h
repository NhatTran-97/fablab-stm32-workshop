#ifndef MULTIPLE_BUTTON_H
#define MULTIPLE_BUTTON_H
#include "stm32l4xx_hal.h"


typedef struct
{
	uint8_t btn_current;
	uint8_t btn_previous;
	uint8_t btn_filter;
	uint8_t is_debouncing;
	uint32_t time_debounce;
	uint32_t time_start_press; // thoi gian bat dau nhan
	uint8_t is_press_timeout;
	//uint8_t button_id;
	
	GPIO_TypeDef *GPIOx;
	uint16_t GPIO_Pin;
}Button_Typedef;


void btn_pressing_callback(Button_Typedef *buttonX);

void btn_release_callback(Button_Typedef *buttonX);

void btn_press_short_callback(Button_Typedef *buttonX);

void btn_press_timeout_callback(Button_Typedef *buttonX);


void button_handle(Button_Typedef *ButtonX);

void button_init(Button_Typedef *buttonX, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);



#endif

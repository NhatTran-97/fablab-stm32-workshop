#ifndef LEDBLINK_BUTTON_H
#define LEDBLINK_BUTTON_H
#include "stm32l4xx_hal.h"
#include "multiple_bt.h"
#include "led_rgb.h"


typedef enum
{
	led_off,
	led_red_blink_1Hz,
	led_green_blink_5hz,
	
} LED_STATE;


typedef struct
{
//	Button_Typedef button1;
//	Button_Typedef button2;
	LED_STATE led_state;
}Blink_Led;

void Led_Red_Blink_1Hz();

void Led_Green_Blink_5Hz();

void Led_Off();



#endif

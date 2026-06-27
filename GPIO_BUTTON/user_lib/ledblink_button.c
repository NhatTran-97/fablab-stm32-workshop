#include "ledblink_button.h"

void Led_Red_Blink_1Hz()
{
	
}
void Led_Green_Blink_5Hz()
{
	
}
void Led_Off()
{
	
}


void led_handle(Blink_Led blink_led)
{
	switch(blink_led.led_state)
		{
			case led_red_blink_1Hz:
				Led_Red_Blink_1Hz();
				break;

			case led_green_blink_5hz:
				Led_Green_Blink_5Hz();
				break;

			case led_off:
				Led_Off();
				break;

			default:
				break;
		}
}

#include "led_rgb.h"

void rgb_init(RGB *rgb, GPIO_TypeDef *gpio_ports[], uint16_t pins[])
{
	rgb->GPIO_red = gpio_ports[0];  // gpio_ports[0] <-> GPIOB
	rgb->GPIO_green = gpio_ports[1]; //gpio_ports[0] <-> GPIOB
	rgb->GPIO_blue = gpio_ports[2]; // gpio_ports[0] <-> GPIOB
	
	rgb->red_pin   = pins[0];  // pins[0] <-> LED_R_Pin
	rgb->green_pin = pins[1]; // pins[1] <-> LED_G_Pin
	rgb->blue_pin  = pins[2]; // pins[2] <-> LED_B_Pin
	
	rgb->state = LOW;
	rgb->color = OFF;
	rgb->last_time_on = HAL_GetTick();

}


void rgb_setcolor(RGB *rgb,Color color)
{
	if(color == OFF) 
	{
		rgb -> state = LOW;
	}
	else
	{
		rgb -> state = HIGH;	
	}		
	switch(color)
	{

		case RED:
			HAL_GPIO_WritePin(rgb -> GPIO_red  , rgb -> red_pin  ,(GPIO_PinState) 1);
			HAL_GPIO_WritePin(rgb -> GPIO_green, rgb -> green_pin,(GPIO_PinState) 0);
			HAL_GPIO_WritePin(rgb -> GPIO_blue , rgb -> blue_pin ,(GPIO_PinState) 0);
			break;
				case GREEN:
			HAL_GPIO_WritePin(rgb -> GPIO_red  , rgb -> red_pin  ,(GPIO_PinState) 0);
			HAL_GPIO_WritePin(rgb -> GPIO_green, rgb -> green_pin,(GPIO_PinState) 1);
			HAL_GPIO_WritePin(rgb -> GPIO_blue , rgb -> blue_pin ,(GPIO_PinState) 0);
		break;	
		case BLUE:
			HAL_GPIO_WritePin(rgb -> GPIO_red  , rgb -> red_pin  ,(GPIO_PinState) 0);
			HAL_GPIO_WritePin(rgb -> GPIO_green, rgb -> green_pin,(GPIO_PinState) 0);
			HAL_GPIO_WritePin(rgb -> GPIO_blue , rgb -> blue_pin ,(GPIO_PinState) 1);
		break;		
		case WHITE:
			HAL_GPIO_WritePin(rgb -> GPIO_red  , rgb -> red_pin  ,(GPIO_PinState) 1);
			HAL_GPIO_WritePin(rgb -> GPIO_green, rgb -> green_pin,(GPIO_PinState) 1);
			HAL_GPIO_WritePin(rgb -> GPIO_blue , rgb -> blue_pin ,(GPIO_PinState) 1);
		break;
		case YELLOW:
			HAL_GPIO_WritePin(rgb -> GPIO_red  , rgb -> red_pin  ,(GPIO_PinState) 1);
			HAL_GPIO_WritePin(rgb -> GPIO_green, rgb -> green_pin,(GPIO_PinState) 1);
			HAL_GPIO_WritePin(rgb -> GPIO_blue , rgb -> blue_pin ,(GPIO_PinState) 0);
		break;	
		case PURPLE:
			HAL_GPIO_WritePin(rgb -> GPIO_red  , rgb -> red_pin  ,(GPIO_PinState) 1);
			HAL_GPIO_WritePin(rgb -> GPIO_green, rgb -> green_pin,(GPIO_PinState) 0);
			HAL_GPIO_WritePin(rgb -> GPIO_blue , rgb -> blue_pin ,(GPIO_PinState) 1);
		break;
		case OFF:
			HAL_GPIO_WritePin(rgb -> GPIO_red  , rgb -> red_pin  ,(GPIO_PinState) 0);
			HAL_GPIO_WritePin(rgb -> GPIO_green, rgb -> green_pin,(GPIO_PinState) 0);
			HAL_GPIO_WritePin(rgb -> GPIO_blue , rgb -> blue_pin ,(GPIO_PinState) 0);
		break;	
		
		default:
			break;
		
	}
	
}

static void rgb_toggle(RGB *rgb, Color color)
{
	if(rgb->state == HIGH)
	{
		rgb_setcolor(rgb, OFF);
		rgb->state = LOW;
	}
	else
	{
		rgb_setcolor(rgb, color);
		rgb->state = HIGH;
		
	}
}

void rgb_blink(RGB *rgb,Color color, uint16_t blink_time)
{
		if (HAL_GetTick() - rgb->last_time_on >= blink_time) 
		{ 
        rgb_toggle(rgb, color);
        rgb->last_time_on = HAL_GetTick();
    }
	
}

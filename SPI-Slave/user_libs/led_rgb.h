#ifndef LED_RGB_H
#define LED_RGB_H
#include "stm32f1xx_hal.h"

#define HIGH 1 
#define LOW 0

typedef enum 
{
	RED, 
	GREEN,
	BLUE,
	WHITE,
	YELLOW,
	PURPLE,
	OFF,
}Color;
typedef struct
{
	GPIO_TypeDef *GPIO_red; // GPIOB
	GPIO_TypeDef *GPIO_green; // GPIOB
	GPIO_TypeDef *GPIO_blue; // GPIOB
	
	uint16_t red_pin;
	uint16_t green_pin;
	uint16_t blue_pin;
	
	uint8_t state;
	uint32_t last_time_on;
	Color color;
	
	
}RGB;


void rgb_init(RGB *rgb, GPIO_TypeDef *gpio_ports[], uint16_t pins[]);
void rgb_setcolor(RGB *rgb,Color color);
static void rgb_toggle(RGB *rgb, Color color);
void rgb_blink(RGB *rgb,Color color, uint16_t blink_time);

#endif

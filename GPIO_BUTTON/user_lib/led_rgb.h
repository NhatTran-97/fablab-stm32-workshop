#ifndef LED_RGB_H
#define LED_RGB_H
#include "stm32l4xx_hal.h"

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

static const uint8_t color_map[] = {
    0x04,  // RED    = 0  (0b100)
    0x02,  // GREEN  = 1  (0b010)
    0x01,  // BLUE   = 2  (0b001)
    0x07,  // WHITE  = 3  (0b111)
    0x06,  // YELLOW = 4  (0b110)
    0x05,  // PURPLE = 5  (0b101)
    0x00,  // OFF    = 6  (0b000)
};

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


//extern RGB *rgb;

void rgb_init( RGB *rgb, GPIO_TypeDef *gpio_ports[], uint16_t pins[]);
void rgb_setcolor( RGB *rgb ,Color color);
void rgb_toggle(RGB *rgb,Color color);
void rgb_blink(RGB *rgb,Color color, uint16_t blink_time);

#endif

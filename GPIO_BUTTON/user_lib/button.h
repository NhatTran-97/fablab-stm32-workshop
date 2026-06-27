#ifndef BUTTON_H
#define BUTTON_H
#include "stm32l4xx_hal.h"

void btn_pressing_callback(void);

void btn_release_callback(void);

void btn_press_short_callback(void);

void btn_press_timeout_callback(void);

void button_handle(void);



#endif

#include "multiple_bt.h"


__weak void btn_pressing_callback(Button_Typedef *buttonX)
{
	
}

__weak void btn_release_callback(Button_Typedef *buttonX)
{
	
}

__weak void btn_press_short_callback(Button_Typedef *buttonX)
{
	
}

__weak void btn_press_timeout_callback(Button_Typedef *buttonX)
{
	
}



void button_handle(Button_Typedef *buttonX)
{
		//uint8_t read_current_staus = HAL_GPIO_ReadPin(GPIOA, BT2_Pin);
		uint8_t read_current_staus = HAL_GPIO_ReadPin(buttonX->GPIOx, buttonX->GPIO_Pin);
		if(read_current_staus !=  buttonX->btn_filter) // Khi co su thay doi trang thai
		{
			buttonX->btn_filter = read_current_staus; // Khi co thay doi -> Cap nhat
			buttonX->is_debouncing = 1;
			buttonX->time_debounce = HAL_GetTick();
		}
		// ---------------TRANG THAI TIN HIEU DA XAC LAP-------------
		if(buttonX->is_debouncing == 1 && HAL_GetTick() - buttonX->time_debounce > 15) // HAL_GetTick() - time_debounce > 15)  15ms sau ma khong co nhieu --> tin hieuj cuoi cung
		{
			buttonX->btn_current = buttonX->btn_filter; // Trang thai su dung
			buttonX->is_debouncing = 0;
		}
		// -----------------XU LY------------
		if(buttonX->btn_current != buttonX->btn_previous)
		{
			if(buttonX->btn_current == 0) // nhan xuong
			{
				buttonX->is_press_timeout = 1;
				btn_pressing_callback(buttonX);
				buttonX->time_start_press = HAL_GetTick(); // Save the time
			}
			else // Nha ra
			{
				if(HAL_GetTick() - buttonX->time_start_press <= 1000)
				{
					btn_press_short_callback(buttonX);
				}
				
				btn_release_callback(buttonX);
			}
			
			buttonX->btn_previous = buttonX->btn_current;
			
		}
		//////// Xu Ly Nhan Giu
		if(buttonX->is_press_timeout && (HAL_GetTick() - buttonX->time_start_press) >= 3000) // sau 3s sau
		{
			btn_press_timeout_callback(buttonX);
			buttonX->is_press_timeout = 0;
		}
}

void button_init(Button_Typedef *buttonX, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin )
{
	buttonX->btn_current = 1;
	buttonX->btn_previous = 1;
	buttonX->btn_filter = 1;
	buttonX->GPIOx = GPIOx;
	buttonX->GPIO_Pin = GPIO_Pin;
}
#include "button.h"
#include "main.h"

//------------------------var button----------------------------------------
uint8_t btn_current = 1;
uint8_t btn_previous = 1;
uint8_t btn_filter = 1;
uint8_t is_debouncing;
uint32_t time_debounce;
uint32_t time_start_press; // thoi gian bat dau nhan
uint8_t is_press_timeout;



void btn_pressing_callback(void)
{
	
}
__weak void btn_release_callback()
{
}
__weak void btn_press_short_callback() 
{
}
__weak void btn_press_timeout_callback()
{
	
}




//extern void btn_press_timeout_callback()
//{
//	
//}


void button_handle()
{
		uint8_t read_current_staus = HAL_GPIO_ReadPin(GPIOA, BT2_Pin);
		// --------- DETECT STATE CHANGE AND START DEBOUNCING ---------
		if(read_current_staus !=  btn_filter)  // A button state transition is detected
		{
			btn_filter = read_current_staus; // Store the latest sampled state
			is_debouncing = 1;
			time_debounce = HAL_GetTick();  // Record the timestamp of the last change
		}
		
		// --------- CONFIRM STABLE STATE AFTER DEBOUNCE INTERVAL ---------
		if(is_debouncing == 1 && HAL_GetTick() - time_debounce > 15)                            
		{
			btn_current = btn_filter;  // Accept the state as valid
			is_debouncing = 0; // Debouncing is complete
		}
		
		// ----------------- BUTTON EVENT HANDLING -----------------
		if (btn_current != btn_previous)
		{
				if (btn_current == 0) // Button press event
				{
						is_press_timeout = 1;
						btn_pressing_callback();
						time_start_press = HAL_GetTick(); // Record the time when the button is pressed
				}
				else // Button release event
				{
						if (HAL_GetTick() - time_start_press <= 1000)
						{
								btn_press_short_callback();   // Valid short press
						}

						btn_release_callback();           // Process button release
				}

				btn_previous = btn_current;           // Store the current state for the next comparison
		}

		// -------- Long press detection --------
		if (is_press_timeout && (HAL_GetTick() - time_start_press) >= 3000) // Held for at least 3 seconds
		{
				btn_press_timeout_callback();         // Trigger long press action
				is_press_timeout = 0;                 // Prevent repeated triggering
		}
}

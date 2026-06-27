#ifndef INC_MOTOR_CONTROL_H_ 
#define INC_MOTOR_CONTROL_H_
#include "main.h"

typedef struct{
	TIM_HandleTypeDef *htim_motor; 	// timer instance for the pwm signal
	uint32_t htim_motor_ch; 				// timer channel 
	GPIO_TypeDef *mdir_pin_port; 		// pin to control the rotation direction
	uint16_t mdir_pin_number; 			//pin to control the rotation direction
	GPIO_TypeDef *rst_pin_port; 		// reset pin
	uint16_t rst_pin_number; 				// reset pin
	float current_duty;  						// last applied duty cycle (-100..100)
	uint8_t is_enabled;  						// 1 if driver is enabled, 0 otherwise
	int8_t direction;  
} motor_inst;


typedef enum {
    MOTOR_OK = 0, 	// success
    MOTOR_CLAMPED, 	// duty cycle was out of range and got clamped
    MOTOR_ERROR 		// failed: motor is NULL or not enabled
} motor_status_t;

void motor_init(motor_inst *motor,
																	 TIM_HandleTypeDef *htim_motor,
																	 uint32_t htim_motor_ch,
																	 GPIO_TypeDef *mdir_pin_port,
																	 uint16_t mdir_pin_number,
																	 GPIO_TypeDef *rst_pin_port,
																	 uint16_t rst_pin_number,
																		int8_t direction);

void enable_motor(motor_inst * motor);
void disable_motor(motor_inst *motor);
motor_status_t set_speed_open(motor_inst *motor, float duty_cycle_percent);
void set_speed_zero(motor_inst *motor);




#endif

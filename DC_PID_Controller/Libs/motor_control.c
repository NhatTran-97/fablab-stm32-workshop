
#include "motor_control.h"


void motor_init(motor_inst *motor,
																	 TIM_HandleTypeDef *htim_motor,
																	 uint32_t htim_motor_ch,
																	 GPIO_TypeDef *mdir_pin_port,
																	 uint16_t mdir_pin_number,
																	 GPIO_TypeDef *rst_pin_port,
																	 uint16_t rst_pin_number,
																		int8_t direction)
{
	  motor->htim_motor       = htim_motor;
    motor->htim_motor_ch    = htim_motor_ch;
    motor->mdir_pin_port    = mdir_pin_port;
    motor->mdir_pin_number  = mdir_pin_number;
    motor->rst_pin_port     = rst_pin_port;
    motor->rst_pin_number   = rst_pin_number;
	  motor-> current_duty = 0.0f;
		motor->direction        = direction;
	
	  HAL_TIM_PWM_Start(motor->htim_motor, motor->htim_motor_ch);
	  __HAL_TIM_SET_COMPARE(motor->htim_motor, motor->htim_motor_ch, 0);
	  enable_motor(motor);
}


void enable_motor(motor_inst * motor)
{
	 HAL_GPIO_WritePin(motor->rst_pin_port, motor->rst_pin_number, GPIO_PIN_SET);
	 motor->is_enabled = 1;
}
void disable_motor(motor_inst *motor)
{
	 HAL_GPIO_WritePin(motor->rst_pin_port, motor->rst_pin_number, GPIO_PIN_RESET);
	 motor->is_enabled = 0;
}

motor_status_t set_speed_open(motor_inst *motor, float duty_cycle_percent)
{
	if(motor == NULL || motor->htim_motor == NULL)
	{
		return MOTOR_ERROR;
	}
	if(!motor->is_enabled)
	{
		return MOTOR_ERROR;
	}
	
	
	duty_cycle_percent *= motor->direction;
	motor_status_t status = MOTOR_OK;
	
	if (duty_cycle_percent > 100.0f)
	{
			duty_cycle_percent = 100.0f;
			status = MOTOR_CLAMPED;
	}
	if (duty_cycle_percent < -100.0f)
	{
			duty_cycle_percent = -100.0f;
			status = MOTOR_CLAMPED;
	}
	 if (duty_cycle_percent == 0.0f)
	{
			set_speed_zero(motor);
			return status;
	}
	motor->current_duty = duty_cycle_percent;
	
	uint32_t arr = motor->htim_motor->Instance->ARR;
	
	 if (duty_cycle_percent > 0.0f)
	{
			uint32_t ccr = (uint32_t)(duty_cycle_percent * (arr + 1) / 100.0f);
			__HAL_TIM_SET_COMPARE(motor->htim_motor, motor->htim_motor_ch, ccr);  // 0->999
			HAL_GPIO_WritePin(motor->mdir_pin_port, motor->mdir_pin_number, GPIO_PIN_SET);
	}
	else
	{
			uint32_t ccr = (uint32_t)(-duty_cycle_percent * (arr + 1) / 100.0f);
			__HAL_TIM_SET_COMPARE(motor->htim_motor, motor->htim_motor_ch, ccr);
			HAL_GPIO_WritePin(motor->mdir_pin_port, motor->mdir_pin_number, GPIO_PIN_RESET);
	}
	return status;

}

void set_speed_zero(motor_inst *motor)
{
	  __HAL_TIM_SET_COMPARE(motor->htim_motor, motor->htim_motor_ch, 0);
    motor->current_duty = 0.0f;
}



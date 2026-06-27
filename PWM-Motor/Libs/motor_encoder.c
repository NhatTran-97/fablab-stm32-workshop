#include "motor_encoder.h"




void encoder_init(encoder_inst *encoder, TIM_HandleTypeDef *htim_encoder, 
                   float timer_period, int8_t direction)
{
	
	  encoder->htim_encoder       = htim_encoder;
    encoder->timer_period       = timer_period;
    encoder->velocity           = 0.0f;
    encoder->position           = 0.0f;
    encoder->last_counter_value = 0;
    encoder->first_time         = 1;
    encoder->zero_velocity_count = 0;
    encoder->direction          = direction;  

    HAL_TIM_Encoder_Start(encoder->htim_encoder, TIM_CHANNEL_ALL);
    __HAL_TIM_SET_COUNTER(encoder->htim_encoder, 0);
}

encoder_status_t get_encoder_speed(encoder_inst *encoder)
{
	  if (encoder == NULL || encoder->htim_encoder == NULL)
    {
        return ENCODER_ERROR;
    }
		int64_t temp_counter = __HAL_TIM_GET_COUNTER(encoder->htim_encoder);
		
		float scaling = TWOPI / (NUMBER_OF_TICKS_PER_REV * (encoder->timer_period));
		
		//encoder->test_count = temp_counter;
		
		if(encoder->first_time)
		{
			encoder->velocity = 0;
			encoder->first_time = 0;
		}
		else
		{
			if (temp_counter == encoder->last_counter_value)
			{
				encoder->velocity = 0;
			}
			else if (temp_counter > encoder->last_counter_value)
			{
				// UNDERFLOW
				if (temp_counter - encoder->last_counter_value > __HAL_TIM_GET_AUTORELOAD(encoder->htim_encoder) / 2)
				{
					 encoder->velocity = scaling * ( -(encoder->last_counter_value) -
																	(__HAL_TIM_GET_AUTORELOAD(encoder->htim_encoder) - temp_counter));
				}
				else // normal
				{
					 encoder->velocity = scaling * (temp_counter - encoder->last_counter_value);
					
				}
			}
			else
			{
				// normal
				if((encoder->last_counter_value - temp_counter) < __HAL_TIM_GET_AUTORELOAD(encoder->htim_encoder) / 2)
				{
					encoder->velocity = scaling * (temp_counter - encoder->last_counter_value);
				}
				else  // OVERFLOW
				{
					 encoder->velocity = scaling * (temp_counter +
																(__HAL_TIM_GET_AUTORELOAD(encoder->htim_encoder) - encoder->last_counter_value));
					
				}
			}
		}
		
		encoder->velocity *= encoder->direction;  // rad/s
		encoder->position += encoder->velocity * encoder->timer_period ;  // rad
		
		encoder->last_counter_value = temp_counter;
		
		 if (encoder->velocity == 0.0f)
    {
        encoder->zero_velocity_count++;
    }
    else
    {
        encoder->zero_velocity_count = 0;
    }
		return ENCODER_OK;
		

}


void reset_encoder(encoder_inst *encoder)
{
	__HAL_TIM_SET_COUNTER(encoder->htim_encoder, 0);
	   encoder->position = 0;
    encoder->first_time = 1;
    encoder->last_counter_value = 0;
    encoder->velocity = 0;
    encoder->zero_velocity_count = 0;
}
#ifndef INC_MOTOR_ENCODER_H_
#define INC_MOTOR_ENCODER_H_
#include "main.h"
#include "moving_average.h"

#define NUMBER_OF_TICKS_PER_REV       1400
#define TWOPI                         6.28318530718f
#define MOVING_AVERAGE_ENCODER 70

typedef struct
{
    float velocity;
    float position;
    int64_t last_counter_value;
    int64_t test_count;
    float timer_period;
    TIM_HandleTypeDef *htim_encoder;
    uint8_t first_time;
    uint32_t zero_velocity_count;
    int8_t direction;      
		maf_instance motor_vel_filter; // filter to smooth the velocity values
} encoder_inst;

typedef enum {
    ENCODER_OK = 0,
    ENCODER_STALLED,
    ENCODER_ERROR
} encoder_status_t;


void encoder_init(encoder_inst *encoder, TIM_HandleTypeDef *htim_encoder, 
                   float timer_period, int8_t direction);

encoder_status_t get_encoder_speed(encoder_inst *encoder);

void reset_encoder(encoder_inst *encoder);


#endif

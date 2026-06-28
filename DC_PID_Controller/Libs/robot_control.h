#ifndef ROBOT_CONTROL_H
#define ROBOT_CONTROL_H

#include "motor_control.h"
#include "motor_encoder.h"
#include "pid_control.h"
#include "usart_telemetry.h"
#include "vel_ramp.h"

typedef enum {
    MODE_VELOCITY,   
    MODE_POSITION,   
} robot_mode_t;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart1;

extern motor_inst motor_left;
extern motor_inst motor_right;


extern encoder_inst enc_motor_left;
extern encoder_inst enc_motor_right;

extern pid_instance pid_motor_left;
extern pid_instance pid_motor_right;
extern pid_instance pid_pos_left;    
extern pid_instance pid_pos_right;   

extern float setpoint_motor_left;
extern float setpoint_motor_right;
extern float pos_setpoint_left;     
extern float pos_setpoint_right;   

extern telem_tx_payload_t telem_data;
extern robot_mode_t robot_mode;  


extern vel_ramp_t ramp_left;
extern vel_ramp_t ramp_right;

// API
void robot_control_init(void);
void robot_control_update(void);      
void robot_control_telem_send(void);  
void robot_control_handle_cmd(void); 

#endif


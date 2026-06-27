#include "robot_control.h"



void robot_control_init(void)
{
    // Motor A
    motor_init(&motor_left, &htim1, TIM_CHANNEL_1,
																							 PHA_GPIO_Port, PHA_Pin,
																							 STSP_GPIO_Port, STSP_Pin, 1);

    // Motor B
    motor_init(&motor_right, &htim1, TIM_CHANNEL_2,
																							 PHB_GPIO_Port, PHB_Pin,
																							 STSP_GPIO_Port, STSP_Pin, -1);

    // Encoder A
    encoder_init(&enc_motor_left, &htim3, 0.002f, +1); 

    // Encoder B
    encoder_init(&enc_motor_right, &htim2, 0.002f, -1); 

    // PID A
    set_pid(&pid_motor_left, 3.0f, 7.0f, 0.01f, 500.0f, 100.0f, 5000.0f);

    // PID B
    set_pid(&pid_motor_right, 3.0f, 5.0f, 0.0f, 500.0f, 100.0f, 5000.0f); 
		
		
		set_pid(&pid_pos_left,  2.0f, 0.0f, 0.0f, 500.0f, 10.0f, 1000.0f);
		set_pid(&pid_pos_right, 2.0f, 0.0f, 0.0f, 500.0f, 10.0f, 1000.0f);
		
		reset_pid(&pid_pos_left);
		reset_pid(&pid_pos_right);

    // Reset
    reset_encoder(&enc_motor_left);
    reset_encoder(&enc_motor_right);  
    reset_pid(&pid_motor_left);
    reset_pid(&pid_motor_right);    

    // Telemetry
    telem_init(&huart1);
}

void robot_control_update(void)
{
    get_encoder_speed(&enc_motor_left);
    get_encoder_speed(&enc_motor_right);

    float vel_sp_left, vel_sp_right;

    // --- Vòng ngoài: Position PID ---------------------------
    if(robot_mode == MODE_POSITION)
    {
        float err_pos_l = pos_setpoint_left  - enc_motor_left.position;
        float err_pos_r = pos_setpoint_right - enc_motor_right.position;

        apply_pid(&pid_pos_left,  err_pos_l);
        apply_pid(&pid_pos_right, err_pos_r);

        vel_sp_left  = pid_pos_left.output;   // output = velocity setpoint
        vel_sp_right = pid_pos_right.output;
    }
    else  // MODE_VELOCITY
    {
        vel_sp_left  = setpoint_motor_left;
        vel_sp_right = setpoint_motor_right;
    }

    // --- Vòng trong: Velocity PID ---------------------------
    float err_vel_l = vel_sp_left  - enc_motor_left.velocity;
    float err_vel_r = vel_sp_right - enc_motor_right.velocity;

    apply_pid(&pid_motor_left,  err_vel_l);
    apply_pid(&pid_motor_right, err_vel_r);

    set_speed_open(&motor_left,  pid_motor_left.output);
    set_speed_open(&motor_right, pid_motor_right.output);

    // --- Update telemetry -----------------------------------
    telem_data.setpoint       = setpoint_motor_left;
    telem_data.velocity       = enc_motor_left.velocity;
    telem_data.position       = enc_motor_left.position;
    telem_data.pid_output     = pid_motor_left.output;
    telem_data.encoder_raw_left = (int32_t)enc_motor_left.last_counter_value;

    telem_data.setpoint_right    = setpoint_motor_right;
    telem_data.velocity_right    = enc_motor_right.velocity;
    telem_data.position_right    = enc_motor_right.position;
    telem_data.pid_output_right  = pid_motor_right.output;
    telem_data.encoder_raw_right = (int32_t)enc_motor_right.last_counter_value;
}


//void robot_control_update(void)
//{
//    get_encoder_speed(&enc_motor_left);
//		get_encoder_speed(&enc_motor_right);
//	
//	
//	
//	

//    float error = setpoint_motor_left - enc_motor_left.velocity;
//    apply_pid(&pid_motor_left, error);
//    set_speed_open(&motor_left, pid_motor_left.output);
//	
//	
//	  
//    float error_b = setpoint_motor_right - enc_motor_right.velocity;
//    apply_pid(&pid_motor_right, error_b);
//    set_speed_open(&motor_right, pid_motor_right.output);

//    // Update telemetry data
//    telem_data.setpoint    = setpoint_motor_left;
//    telem_data.velocity    = enc_motor_left.velocity;
//    telem_data.position    = enc_motor_left.position;
//    telem_data.pid_output  = pid_motor_left.output;
//    telem_data.encoder_raw_left = (int32_t)enc_motor_left.last_counter_value;
//	
//	
//		telem_data.setpoint_right     = setpoint_motor_right;
//		telem_data.velocity_right     = enc_motor_right.velocity;
//		telem_data.position_right     = enc_motor_right.position;
//		telem_data.pid_output_right   = pid_motor_right.output;
//		telem_data.encoder_raw_right  = (int32_t)enc_motor_right.last_counter_value;
//}

void robot_control_telem_send(void)
{
    telem_send(&telem_data);
}

void robot_control_handle_cmd(void)
{
    telem_rx_payload_t cmd;
	
    if(telem_get_cmd(&cmd))
    {
        setpoint_motor_left  = cmd.setpoint_left;
        setpoint_motor_right = cmd.setpoint_right;
        
        set_pid(&pid_motor_left,  cmd.kp_left,  cmd.ki_left,  cmd.kd_left, 500.0f, 999.0f, 5000.0f);
			
        set_pid(&pid_motor_right, cmd.kp_right, cmd.ki_right, cmd.kd_right,500.0f, 999.0f, 5000.0f);
    }
}

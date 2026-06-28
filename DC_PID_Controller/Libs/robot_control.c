#include "robot_control.h"
#include <math.h>
uint32_t last_cmd_time = 0;





extern telem_rx_payload_t debug_last_cmd;


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

    encoder_init(&enc_motor_left, &htim3, 0.002f, +1); 
    encoder_init(&enc_motor_right, &htim2, 0.002f, -1); 
	
	
    set_pid(&pid_motor_left, 3.0f, 5.0f, 0.02f, 500.0f, 100.0f, 5000.0f);
    set_pid(&pid_motor_right, 3.0f, 5.0f, 0.02f, 500.0f, 100.0f, 5000.0f); 
		set_pid(&pid_pos_left,  7.0f, 0.0f, 0.1f, 50.0f, 10.0f, 1000.0f);
		set_pid(&pid_pos_right, 7.0f, 0.0f, 0.1f, 50.0f, 10.0f, 1000.0f);
		

		
		
		    // Reset
    reset_encoder(&enc_motor_left);
    reset_encoder(&enc_motor_right); 
		
		reset_pid(&pid_pos_left);
		reset_pid(&pid_pos_right);
    reset_pid(&pid_motor_left);
    reset_pid(&pid_motor_right);    
		
		// Init ramp
		vel_ramp_init(&ramp_left,  20.0f, 0.5f, 3.3f); //// max=20, accel=0.3s, decel=0.3s
		vel_ramp_init(&ramp_right, 20.0f, 0.5f, 3.3f);

    // Telemetry
    telem_init(&huart1);
}

void robot_control_update(void)
{
	   static uint8_t pos_counter = 0;
    get_encoder_speed(&enc_motor_left);
    get_encoder_speed(&enc_motor_right);

    float vel_sp_left, vel_sp_right;
	
		vel_ramp_update(&ramp_left,  0.002f);
		vel_ramp_update(&ramp_right, 0.002f);

    // --- Vòng ngoài: Position PID ---------------------------
	

    if(robot_mode == MODE_POSITION)
    {
        if(++pos_counter >= 10)  
        {
            pos_counter = 0;
            float err_pos_l = pos_setpoint_left  - enc_motor_left.position;
            float err_pos_r = pos_setpoint_right - enc_motor_right.position;
            apply_pid(&pid_pos_left,  err_pos_l);
            apply_pid(&pid_pos_right, err_pos_r);
        }
        vel_sp_left  = pid_pos_left.output;
        vel_sp_right = pid_pos_right.output;
    }
    else  // MODE_VELOCITY
    {
			  pos_counter  = 0;
//        vel_sp_left  = setpoint_motor_left;
//        vel_sp_right = setpoint_motor_right;
			
			  vel_sp_left  = ramp_left.current_vel;   // ? dùng ramp output
        vel_sp_right = ramp_right.current_vel;  // ? dùng ramp output
    }

    // --- Vòng trong: Velocity PID ---------------------------
    float err_vel_l = vel_sp_left  - enc_motor_left.velocity;
    float err_vel_r = vel_sp_right - enc_motor_right.velocity;

    apply_pid(&pid_motor_left,  err_vel_l);
    apply_pid(&pid_motor_right, err_vel_r);

    set_speed_open(&motor_left,  pid_motor_left.output);
    set_speed_open(&motor_right, pid_motor_right.output);

    // --- Update telemetry -----------------------------------
   // telem_data.setpoint       = setpoint_motor_left;
		telem_data.setpoint       = ramp_left.current_vel; 
    telem_data.velocity       = enc_motor_left.velocity;
    telem_data.position       = enc_motor_left.position;
    telem_data.pid_output     = pid_motor_left.output;
    telem_data.encoder_raw_left = (int32_t)enc_motor_left.last_counter_value;

    //telem_data.setpoint_right    = setpoint_motor_right;
		telem_data.setpoint_right = ramp_right.current_vel; 
    telem_data.velocity_right    = enc_motor_right.velocity;
    telem_data.position_right    = enc_motor_right.position;
    telem_data.pid_output_right  = pid_motor_right.output;
    telem_data.encoder_raw_right = (int32_t)enc_motor_right.last_counter_value;
		
		telem_data.mode              = (uint8_t)robot_mode;
		telem_data.pos_setpoint_left  = pos_setpoint_left;
		telem_data.pos_setpoint_right = pos_setpoint_right;
}



void robot_control_telem_send(void)
{
    telem_send(&telem_data);
}




void robot_control_handle_cmd(void)
{
    telem_rx_payload_t cmd;
    
    if(telem_get_cmd(&cmd))
    {
        debug_last_cmd = cmd;
        setpoint_motor_left  = cmd.setpoint_left;
        setpoint_motor_right = cmd.setpoint_right;

//        set_pid(&pid_motor_left,  cmd.kp_left,  cmd.ki_left,  cmd.kd_left, 500.0f, 999.0f, 5000.0f);
//        set_pid(&pid_motor_right, cmd.kp_right, cmd.ki_right, cmd.kd_right,500.0f, 999.0f, 5000.0f);

//        set_pid(&pid_pos_left,  cmd.pos_kp_left,  cmd.pos_ki_left,  cmd.pos_kd_left, 50.0f, 10.0f, 1000.0f);
//        set_pid(&pid_pos_right, cmd.pos_kp_right, cmd.pos_ki_right, cmd.pos_kd_right, 50.0f, 10.0f, 1000.0f);
			
			
			    set_pid_gains(&pid_motor_left,  cmd.kp_left,  cmd.ki_left,  cmd.kd_left,
                  500.0f, 100.0f, 5000.0f);
					set_pid_gains(&pid_motor_right, cmd.kp_right, cmd.ki_right, cmd.kd_right,
												500.0f, 100.0f, 5000.0f);
					set_pid_gains(&pid_pos_left,  cmd.pos_kp_left,  cmd.pos_ki_left,  cmd.pos_kd_left,
												50.0f, 10.0f, 1000.0f);
					set_pid_gains(&pid_pos_right, cmd.pos_kp_right, cmd.pos_ki_right, cmd.pos_kd_right,
												50.0f, 10.0f, 1000.0f);

        if(fabsf(cmd.setpoint_left) < 0.05f && fabsf(cmd.setpoint_right) < 0.05f)
        {

					 
        
            vel_ramp_set_target(&ramp_left,  0.0f);
            vel_ramp_set_target(&ramp_right, 0.0f);

         
            if(fabsf(ramp_left.current_vel)  < 0.5f &&
               fabsf(ramp_right.current_vel) < 0.5f)
            {
                robot_mode = MODE_POSITION;
                pos_setpoint_left  = enc_motor_left.position;
                pos_setpoint_right = enc_motor_right.position;

                reset_pid(&pid_motor_left);
                reset_pid(&pid_motor_right);
                reset_pid(&pid_pos_left);
                reset_pid(&pid_pos_right);
                vel_ramp_reset(&ramp_left);
                vel_ramp_reset(&ramp_right);
            }
            else
            {
                robot_mode = MODE_VELOCITY;  
            }
        }
        else
        {
            robot_mode = MODE_VELOCITY;
            vel_ramp_set_target(&ramp_left,  cmd.setpoint_left);
            vel_ramp_set_target(&ramp_right, cmd.setpoint_right);
        }
    }
}


//void robot_control_handle_cmd(void)
//{
//	telem_rx_payload_t cmd;
//	
//	if(telem_get_cmd(&cmd))
//	{
//		debug_last_cmd = cmd;

//		setpoint_motor_left = cmd.setpoint_left;
//		setpoint_motor_right = cmd.setpoint_right;
//		
//		set_pid(&pid_motor_left,  cmd.kp_left,  cmd.ki_left,  cmd.kd_left,
//                500.0f, 999.0f, 5000.0f);
//    set_pid(&pid_motor_right, cmd.kp_right, cmd.ki_right, cmd.kd_right,
//                500.0f, 999.0f, 5000.0f);
//		
//		set_pid(&pid_pos_left,  cmd.pos_kp_left,  cmd.pos_ki_left,  cmd.pos_kd_left,
//                50.0f, 10.0f, 1000.0f);
//    set_pid(&pid_pos_right, cmd.pos_kp_right, cmd.pos_ki_right, cmd.pos_kd_right,
//                50.0f, 10.0f, 1000.0f);

//		
//		if(fabsf(cmd.setpoint_left) < 0.05f && fabsf(cmd.setpoint_right) < 0.05f)
//		{
//			// Lenh dung --> Giu vi tri
//			robot_mode = MODE_POSITION;
//			pos_setpoint_left = enc_motor_left.position;
//			pos_setpoint_right = enc_motor_right.position;
//			
//			reset_pid(&pid_motor_left);
//			reset_pid(&pid_motor_right);
//			reset_pid(&pid_pos_left);
//			reset_pid(&pid_pos_right);
//			vel_ramp_reset(&ramp_left);   
//      vel_ramp_reset(&ramp_right);  
//		}
//		else
//		{
//			robot_mode = MODE_VELOCITY;
//			
//			vel_ramp_set_target(&ramp_left,  cmd.setpoint_left);
//			vel_ramp_set_target(&ramp_right, cmd.setpoint_right);
//		}
//	}
//	

//}


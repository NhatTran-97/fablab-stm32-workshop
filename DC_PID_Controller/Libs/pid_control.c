#include "pid_control.h"


void set_pid(pid_instance *pid, float p, float i, float d,
             float sam_rate, float pid_max, float integral_max)
{
    pid->error_integral = 0;
    pid->last_error     = 0;
    pid->output         = 0;
    pid->p_gain         = p;
    pid->i_gain         = i;
    pid->d_gain         = d;
    pid->sam_rate       = sam_rate;
    pid->pid_max        = pid_max;
    pid->integral_max   = integral_max;
}


void set_pid_gains(pid_instance *pid, float p, float i, float d,
                   float sam_rate, float pid_max, float integral_max)
{
    // Ch? update gains, KHÔNG reset state
    pid->p_gain       = p;
    pid->i_gain       = i;
    pid->d_gain       = d;
    pid->sam_rate     = sam_rate;
    pid->pid_max      = pid_max;
    pid->integral_max = integral_max;
   
}

void reset_pid(pid_instance *pid)
{
	pid->error_integral = 0;
	pid->last_error = 0;
}

/* @brief apply_pid
	* This function computes the PID output considering the PID gains 
	*	@param pid: pid_instance
	* @param input_error: input error
	* @retval: none
	
*/


//pid_typedef apply_pid(pid_instance *pid, float input_error)
//{
//	pid ->error_integral += input_error;
//	if(pid->error_integral > pid ->integral_max)
//	{
//		pid->error_integral = pid ->integral_max;
//	}
//	if(pid->error_integral < -pid ->integral_max)
//	{
//		pid->error_integral = -pid ->integral_max;
//	}
//	if ( pid ->sam_rate == 0)
//	{
//		return pid_numerical;
//	}
//	pid ->output = pid ->p_gain * input_error +
//																							pid ->i_gain * (pid->error_integral) / pid ->sam_rate +
//																							pid ->d_gain * pid ->sam_rate * (input_error - pid->last_error);

//	if(pid->output >= pid ->pid_max)
//	{
//		pid->output = pid ->pid_max;
//	}
//	if(pid->output <= -pid ->pid_max)
//	{
//		pid->output = -pid ->pid_max;
//	}
//	pid->last_error = input_error;

//	return pid_ok;
//}

pid_typedef apply_pid(pid_instance *pid, float input_error)
{
    if(pid->sam_rate == 0)
		{
			return pid_numerical;
		}
        
        
    float p_term = pid->p_gain * input_error;
    float d_term = pid->d_gain * pid->sam_rate * (input_error - pid->last_error);
    float i_term = pid->i_gain * pid->error_integral / pid->sam_rate;
    

    if((p_term + i_term + d_term) < pid->pid_max && 
       (p_term + i_term + d_term) > -pid->pid_max)
    {
        pid->error_integral += input_error;
        if(pid->error_integral > pid->integral_max)
            pid->error_integral = pid->integral_max;
        if(pid->error_integral < -pid->integral_max)
            pid->error_integral = -pid->integral_max;
            
       
        i_term = pid->i_gain * pid->error_integral / pid->sam_rate;
    }
    
    pid->output = p_term + i_term + d_term;  
    
    if(pid->output > pid->pid_max)
        pid->output = pid->pid_max;
    if(pid->output < -pid->pid_max)
        pid->output = -pid->pid_max;
        
    pid->last_error = input_error;
    return pid_ok;
}

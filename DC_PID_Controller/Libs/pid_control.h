#ifndef INC_PID_CONTROL_H_ 
#define INC_PID_CONTROL_H_ 

#include "main.h"

typedef enum {
    pid_ok,
    pid_numerical
} pid_typedef;

typedef struct {
    float p_gain;
    float i_gain;
    float d_gain;
    float sam_rate;       // t?n s? l?y m?u (Hz), ví d? 100.0f cho 10ms
    float error_integral;
    float integral_max;
    float last_error;
    float output;
    float pid_max;
} pid_instance;

pid_typedef apply_pid(pid_instance *pid, float input_error);
void reset_pid(pid_instance *pid);
void set_pid(pid_instance *pid, float p, float i, float d,
             float sam_rate, float pid_max, float integral_max);

#endif  // INC_PID_CONTROL_H_

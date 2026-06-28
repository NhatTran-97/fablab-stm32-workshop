#ifndef VEL_RAMP_H
#define VEL_RAMP_H

#include "main.h"
#include <math.h>

typedef struct {
    float max_vel;      // rad/s t?i da
    float accel_time;   // giây tang t? 0 ? max
    float decel_time;   // giây gi?m t? max ? 0
    float accel_rate;   // rad/s² = max_vel / accel_time
    float decel_rate;   // rad/s² = max_vel / decel_time
    float current_vel;  // velocity hi?n t?i sau ramp
    float target_vel;   // velocity mong mu?n
} vel_ramp_t;

void vel_ramp_init(vel_ramp_t *ramp, float max_vel, float accel_time, float decel_time);
void vel_ramp_update(vel_ramp_t *ramp, float dt);
void vel_ramp_set_target(vel_ramp_t *ramp, float target);
void vel_ramp_reset(vel_ramp_t *ramp);

#endif
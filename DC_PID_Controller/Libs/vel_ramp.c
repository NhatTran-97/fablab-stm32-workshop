#include "vel_ramp.h"

void vel_ramp_init(vel_ramp_t *ramp, float max_vel,
                   float accel_time, float decel_time)
{
    ramp->max_vel    = max_vel;
    ramp->accel_time = accel_time;
    ramp->decel_time = decel_time;
    ramp->accel_rate = max_vel / accel_time;
    ramp->decel_rate = max_vel / decel_time;
    ramp->current_vel = 0.0f;
    ramp->target_vel  = 0.0f;
}

void vel_ramp_set_target(vel_ramp_t *ramp, float target)
{
    // Clamp target
    if(target >  ramp->max_vel) target =  ramp->max_vel;
    if(target < -ramp->max_vel) target = -ramp->max_vel;
    ramp->target_vel = target;
}

void vel_ramp_update(vel_ramp_t *ramp, float dt)
{
    float diff = ramp->target_vel - ramp->current_vel;

    if(fabsf(diff) < 0.01f)
    {
        ramp->current_vel = ramp->target_vel;
        return;
    }

    float rate;
    if(fabsf(ramp->target_vel) >= fabsf(ramp->current_vel))
        rate = ramp->accel_rate;  // tang t?c
    else
        rate = ramp->decel_rate;  // gi?m t?c

    float max_change = rate * dt;

    if(diff > 0)
        ramp->current_vel += fminf(diff,  max_change);
    else
        ramp->current_vel += fmaxf(diff, -max_change);
}

void vel_ramp_reset(vel_ramp_t *ramp)
{
    ramp->current_vel = 0.0f;
    ramp->target_vel  = 0.0f;
}
#ifndef __CONTROL_H
#define __CONTROL_H

#include "stdint.h"
#include "global.h"

typedef struct {
    float Kp, Ki;   // 只留 PI
    float integral; // 积分项
} PID_t;

// === 1. 机械零点 (你之前测出来的那个值) ===
// 假设你测出来平衡时是 -4.0 度，这里就填 -4.0f
// 如果还没测，先填 0，等会儿 PID 跑起来你会发现车往一边跑，再回来改
#define MECHANICAL_ZERO_ANGLE  -1.7 

void PID_Init(PID_t *pid);

// === 2. 函数声明 ===
// int Vertical_Ring(float Pitch, float Gyro_Rate_DegPerSec);
// int Velocity_Ring(int Target_Speed, int Actual_Speed);
// int Turn_Ring(float Target_Turn_Rate, float Gyro_Z_Rate);

float Velocity_Ring(PID_t *p, float Target_Speed, float Actual_Speed);

float Amplitude_Limit(float value, float max_limit);

#endif
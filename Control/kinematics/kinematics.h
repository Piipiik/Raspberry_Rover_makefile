#ifndef __KINEMATICS_H__
#define __KINEMATICS_H__

#include <stdint.h>
#include "global.h"
#include "motor.h"

typedef struct {
    float wheel_base;           // 前后轴距 L
    float track_width;          // 左右轮距 W
    float l_sum;                // (L+W)/2，计算旋转用的几何常数
    
    struct {
        float per_pulse_distance; // 每个脉冲代表的位移 (105.805那个系数)
        uint16_t last_tick;        // 上一次的编码器数值
        float current_speed;      // 计算出的实时线速度 (mm/s)
    } motor[4];                   // 0:LF, 1:RF, 2:LR, 3:RR
} Kinematics_t;

extern uint16_t ticks[MOTOR_COUNT];
extern float speeds[MOTOR_COUNT];

void Kinematics_Init(Kinematics_t *kin);
void Kinematics_SetWheelDistance(Kinematics_t *kin, float wheel_base, float track_width);
void Kinematics_SetMotorParam(Kinematics_t *kin, MotorID_t id, float per_pulse_distance);
void Kinematics_Inverse(Kinematics_t *kin, float vx, float vy, float wz, float *speeds);
void Kinematics_Forward(Kinematics_t *kin, float *vx, float *vy, float *wz);
void Kinematics_UpdateMotorSpeed(Kinematics_t *kin, Motor_t *Motor);
int16_t Kinematics_GetMotorSpeed(Kinematics_t *kin, MotorID_t id);

#endif /* __KINEMATICS_H__ */
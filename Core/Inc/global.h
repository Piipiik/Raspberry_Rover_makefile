/* global.h */
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "stdint.h"

typedef enum {
    MOTOR_LF = 0, // Left Front  左前
    MOTOR_RF = 1, // Right Front 右前
    MOTOR_LR = 2, // Left Rear   左后
    MOTOR_RR = 3, // Right Rear  右后
    MOTOR_COUNT   // 自动计数，这里等于 4
} MotorID_t;

#endif
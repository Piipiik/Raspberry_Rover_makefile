/* App/app.h */
#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_config.h"
#include <stdint.h>
#include "global.h"
#include "bsp_soft_i2c.h"
#include "kinematics.h"
#include "pid.h"
#include "cmsis_os.h"

/* ================= 引用 freertos.c 里的全局变量 ================= */
/* 注意：MOTOR_COUNT, PID_t, Motor_t 这些类型定义要确保能被找到，
   建议把它们也扔到 app.h 或者 global.h 里，这里假设它们已经在 global.h 里定义好了 */

extern Kinematics_t myKin;
extern PID_t pid[MOTOR_COUNT];
extern Motor_t Motor[MOTOR_COUNT]; // 电机数组，索引对应各个电机
extern float target_wheel_speed[MOTOR_COUNT]; // 目标轮速数组，索引对应各个电机
extern float target_vx, target_vy, target_wz;
extern float current_vx, current_vy, current_wz;

extern uint16_t encoder_ticks[]; 
extern uint32_t GetTaskStackWaterMark;
extern float pwm_output[];

extern uint8_t g_mpu_id;
extern SoftI2C_Type OLED_I2C;
extern SoftI2C_Type MPU6050_I2C;

/* ================= 包含各个模块的头文件 ================= */
#include "Motion/app_motion.h"
#include "Sensor/app_sensor.h"
#include "Interact/app_interact.h"
#include "Display/app_display.h"
#include "Monitor/app_monitor.h"

#ifdef __cplusplus
}
#endif

#endif /* __APP_H */
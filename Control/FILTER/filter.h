#ifndef __FILTER__H
#define __FILTER__H

#include <stdint.h>

extern float Gyro_Y_Offset;
extern float Pitch_Angle;

void Filter_Init(void);

// 计算加速度计的倾角 (结果是度)
// 参数：ax, az 是 MPU6050 读出来的原始数据 (int16_t)
float Get_Accel_Angle(int16_t ax, int16_t az);

// 计算陀螺仪角速度 (结果是 度/秒)
// 参数：gx 是 MPU6050 读出来的原始数据
float Get_Gyro_Rate(int16_t gx);
float Get_Gyro_Rate_gz(int16_t gz);

// 校准函数
void MPU6050_Calibrate_Gyro(void);
void MPU6050_Calibrate_Gyro_gz(void);

// 姿态解算函数
void MPU6050_Update_Posture(int16_t ay, int16_t az, int16_t gx);

#endif
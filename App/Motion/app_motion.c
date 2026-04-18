/* App/Motion/app_motion.c */
#include "app_motion.h"
#include "app.h" // 包含总头文件，为了能访问 target_vx 等全局变量
#include "kinematics.h"
#include "motor.h"
#include "pid.h"

Kinematics_t myKin;
PID_t pid[MOTOR_COUNT] = {0};
Motor_t Motor[MOTOR_COUNT] = {0}; // 电机数组，索引对应各个电机
float target_wheel_speed[MOTOR_COUNT] = {0}; // 目标轮速数组，索引对应各个电机
float target_vx = 0.1f, target_vy = 0.0f, target_wz = 0.0f;
float current_vx = 0.0f, current_vy = 0.0f, current_wz = 0.0f;
float pwm_output[MOTOR_COUNT] = {0.0f};


void MotorControlTask(void *argument)
{
  /* 这里直接粘贴你原来 freertos.c 里 MotorControlTask 的所有代码 */
  
  Kinematics_Init(&myKin);
  Motor_Init(Motor); // 注意：这里的 Motor 数组也是 extern 的，确保在 app.h 里声明了
  PID_Init(pid);

  uint32_t tick = osKernelGetTickCount();

  for(;;)
  {
    tick = tick + 10;

    /* --- 以下是你原来的代码，原样保留 --- */
    
    Kinematics_UpdateMotorSpeed(&myKin, Motor);
    Kinematics_Forward(&myKin, &current_vx, &current_vy, &current_wz);
    Kinematics_Inverse(&myKin, target_vx, target_vy, target_wz, target_wheel_speed);

    for(int i = 0; i < MOTOR_COUNT; i++) {
        pwm_output[i] = Velocity_Ring(&pid[i], target_wheel_speed[i], myKin.motor[i].current_speed);
        Motor_SetPWM(&Motor[i], pwm_output[i]);
    }
    osDelayUntil(tick);
  }
}
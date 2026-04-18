#ifndef __MOTOR_H
#define __MOTOR_H

#include "tim.h"
#include "gpio.h"
#include "global.h"
#include <stdint.h>

// 定义 PWM 的最大值 (根据你的定时器 ARR 决定)
#define PWM_MAX 3500

/* ============================================================
   [核心结构体] 电机对象封装
   ============================================================ */
typedef struct {
    // --- 1. 硬件绑定层 (底层资源) ---
    TIM_HandleTypeDef *pwm_tim; // PWM定时器句柄 (如 &htim1)
    uint32_t pwm_channel;       // PWM通道 (如 TIM_CHANNEL_1)
    
    TIM_HandleTypeDef *enc_tim; // 编码器定时器句柄 (如 &htim2)
    
    GPIO_TypeDef *dir_port;     // 方向引脚所在的端口 (如 GPIOB)
    uint16_t pin_IN1;           // 方向控制引脚1 (正转)
    uint16_t pin_IN2;           // 方向控制引脚2 (反转)

    // --- 2. 参数层 (特性调校) ---
    int dead_zone;              // 电机死区补偿值 (克服静摩擦力)
    int polarity;               // 极性翻转标志 (1 或 -1)，用于修正编码器或电机接线反向
    
    // --- 3. 运行状态层 (数据交互) ---
    int16_t current_speed;      // 当前速度 (编码器读取值)
    int out_pwm;                // 最终计算出的 PWM 输出值
} Motor_t;

/* ============================================================
   [函数接口]
   ============================================================ */

void Motor_Init(Motor_t *Motor);
void Motor_UpdateSpeed(Motor_t *motor);
void Motor_SetPWM(Motor_t *motor, int pwm_val);

#endif
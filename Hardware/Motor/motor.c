#include "motor.h"


// 内部辅助函数：求绝对值
static int abs_int(int val) {
    return (val < 0) ? -val : val;
}

/* ============================================================
   [底层初始化] 将硬件资源绑定到对应的电机结构体上
   ============================================================ */
void Motor_Init(Motor_t *Motor)
{
    // --------------------------------------------------------
    // 1. 左前轮 (LF) 配置
    // --------------------------------------------------------
    Motor[MOTOR_LF].pwm_tim     = &htim8;
    Motor[MOTOR_LF].pwm_channel = TIM_CHANNEL_2;
    Motor[MOTOR_LF].enc_tim     = &htim3;            // 假设用 TIM2 读编码器
    Motor[MOTOR_LF].dir_port    = GPIOB;
    Motor[MOTOR_LF].pin_IN1     = GPIO_PIN_4;
    Motor[MOTOR_LF].pin_IN2     = GPIO_PIN_5;
    Motor[MOTOR_LF].dead_zone   = 300;
    Motor[MOTOR_LF].polarity    = -1;                 // 如果发现正转时速度为负，改成 -1

    // --------------------------------------------------------
    // 2. 右前轮 (RF) 配置
    // --------------------------------------------------------
    Motor[MOTOR_RF].pwm_tim     = &htim8;
    Motor[MOTOR_RF].pwm_channel = TIM_CHANNEL_1;
    Motor[MOTOR_RF].enc_tim     = &htim2;            // 假设用 TIM3 读编码器
    Motor[MOTOR_RF].dir_port    = GPIOA;
    Motor[MOTOR_RF].pin_IN1     = GPIO_PIN_11;
    Motor[MOTOR_RF].pin_IN2     = GPIO_PIN_8;
    Motor[MOTOR_RF].dead_zone   = 400;
    Motor[MOTOR_RF].polarity    = 1;                // 麦轮通常左右对称安装，右侧往往需要反向

    // --------------------------------------------------------
    // 3. 左后轮 (LR) 配置
    // --------------------------------------------------------
    Motor[MOTOR_LR].pwm_tim     = &htim8;
    Motor[MOTOR_LR].pwm_channel = TIM_CHANNEL_4;
    Motor[MOTOR_LR].enc_tim     = &htim5;            // 假设用 TIM4 读编码器
    Motor[MOTOR_LR].dir_port    = GPIOC;             // 假设用到 PC 口
    Motor[MOTOR_LR].pin_IN1     = GPIO_PIN_4;
    Motor[MOTOR_LR].pin_IN2     = GPIO_PIN_5;
    Motor[MOTOR_LR].dead_zone   = 250;
    Motor[MOTOR_LR].polarity    = -1;

    // --------------------------------------------------------
    // 4. 右后轮 (RR) 配置
    // --------------------------------------------------------
    Motor[MOTOR_RR].pwm_tim     = &htim8;
    Motor[MOTOR_RR].pwm_channel = TIM_CHANNEL_3;
    Motor[MOTOR_RR].enc_tim     = &htim4;            // 假设用 TIM5 读编码器
    Motor[MOTOR_RR].dir_port    = GPIOC;
    Motor[MOTOR_RR].pin_IN1     = GPIO_PIN_3;
    Motor[MOTOR_RR].pin_IN2     = GPIO_PIN_2;
    Motor[MOTOR_RR].dead_zone   = 350;
    Motor[MOTOR_RR].polarity    = 1;

    // --------------------------------------------------------
    // 5. 启动所有外设 (PWM 与 编码器模式)
    // --------------------------------------------------------
    // 开启电机驱动板的总体使能引脚 (STBY/EN)，如果有的话
    // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); 

    // 启动 4路 编码器之前，强制把硬件计数器清零！
    __HAL_TIM_SET_COUNTER(Motor[MOTOR_LF].enc_tim, 0);
    __HAL_TIM_SET_COUNTER(Motor[MOTOR_RF].enc_tim, 0);
    __HAL_TIM_SET_COUNTER(Motor[MOTOR_LR].enc_tim, 0);
    __HAL_TIM_SET_COUNTER(Motor[MOTOR_RR].enc_tim, 0);

    // 启动 4路 PWM
    HAL_TIM_PWM_Start(Motor[MOTOR_LF].pwm_tim, Motor[MOTOR_LF].pwm_channel);
    HAL_TIM_PWM_Start(Motor[MOTOR_RF].pwm_tim, Motor[MOTOR_RF].pwm_channel);
    HAL_TIM_PWM_Start(Motor[MOTOR_LR].pwm_tim, Motor[MOTOR_LR].pwm_channel);
    HAL_TIM_PWM_Start(Motor[MOTOR_RR].pwm_tim, Motor[MOTOR_RR].pwm_channel);

    // 启动 4路 编码器
    HAL_TIM_Encoder_Start(Motor[MOTOR_LF].enc_tim, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(Motor[MOTOR_RF].enc_tim, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(Motor[MOTOR_LR].enc_tim, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(Motor[MOTOR_RR].enc_tim, TIM_CHANNEL_ALL);
}

/* ============================================================
   [测速函数] 传入指定电机，读取它的速度并清零计数器
   ============================================================ */
void Motor_UpdateSpeed(Motor_t *motor)
{
    // 1. 读取计数值 (强转为有符号的16位整数，利用溢出特性处理正负)
    int16_t speed = (int16_t)__HAL_TIM_GET_COUNTER(motor->enc_tim);
    
    // 2. 清零计数器，为下一个周期做准备
    __HAL_TIM_SET_COUNTER(motor->enc_tim, 0);
    
    // 3. 乘以极性标志，修正因为硬件安装导致的反向问题，存入结构体
    motor->current_speed = speed * motor->polarity; 
}

/* ============================================================
   [驱动函数] 传入指定电机和目标PWM，执行驱动
   ============================================================ */
void Motor_SetPWM(Motor_t *motor, int pwm_val)
{
    int final_pwm = 0;

    // 同样乘以极性标志，保证代码给正数车就往前走
    // pwm_val = pwm_val * motor->polarity; 

    // 1. 方向与死区处理
    if (pwm_val > 0) // 正转
    {
        HAL_GPIO_WritePin(motor->dir_port, motor->pin_IN1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(motor->dir_port, motor->pin_IN2, GPIO_PIN_RESET);
        // final_pwm = pwm_val + motor->dead_zone;
        final_pwm = (pwm_val < motor->dead_zone) ? motor->dead_zone : pwm_val;
    }
    else if (pwm_val < 0) // 反转
    {
        HAL_GPIO_WritePin(motor->dir_port, motor->pin_IN1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(motor->dir_port, motor->pin_IN2, GPIO_PIN_SET);
        // final_pwm = abs_int(pwm_val) + motor->dead_zone;
        final_pwm = (abs_int(pwm_val) < motor->dead_zone) ? motor->dead_zone : abs_int(pwm_val);
    }
    else // 停止
    {
        HAL_GPIO_WritePin(motor->dir_port, motor->pin_IN1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(motor->dir_port, motor->pin_IN2, GPIO_PIN_RESET);
        final_pwm = 0;
    }

    // 2. 限幅保护，防止 PWM 超过定时器 ARR 设定值导致电平翻转乱套
    if (final_pwm > PWM_MAX) 
    {
        final_pwm = PWM_MAX;
    }

    // 3. 记录当前输出并写入寄存器
    motor->out_pwm = final_pwm;
    __HAL_TIM_SET_COMPARE(motor->pwm_tim, motor->pwm_channel, final_pwm);
}
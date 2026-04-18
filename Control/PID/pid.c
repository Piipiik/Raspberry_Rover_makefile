#include "pid.h"
#include "main.h"
#include <math.h>

void PID_Init(PID_t *pid)
{
    pid[MOTOR_LF].Kp = 1200.0f;
    pid[MOTOR_LF].Ki = 30000.0f;
    pid[MOTOR_LF].integral = 0;

    pid[MOTOR_LR].Kp = 1200.0f;
    pid[MOTOR_LR].Ki = 30000.0f;
    pid[MOTOR_LR].integral = 0;

    pid[MOTOR_RF].Kp = 1200.0f;
    pid[MOTOR_RF].Ki = 30000.0f;
    pid[MOTOR_RF].integral = 0;

    pid[MOTOR_RR].Kp = 1200.0f;
    pid[MOTOR_RR].Ki = 30000.0f; // 给后轮加点积分，帮助消除稳态误差
    pid[MOTOR_RR].integral = 0;
}

// === 全局 PID 参数 (方便在 debug 模式下实时修改) ===
// 经验值：GA25电机 + 12V
// Kp: 范围通常在 -100 到 -500 (或者是正数，取决于你的角度极性)
// Kd: 范围通常在 Kp 的 0.01 到 0.05 倍之间
// float Vertical_Kp = -0.0f; // 先给个小一点的值，正负号等会儿测
// float Vertical_Kd = -0.0f;   

// === 直立环 PD 控制 ===
// 输入：Pitch (当前角度), Gyro_Y (Y轴角速度)
// 输出：PWM 期望值
// int Vertical_Ring(float Pitch, float Gyro_Rate_DegPerSec)
// {
//     // 1. 计算角度偏差
//     float Angle_Err = Pitch - MECHANICAL_ZERO_ANGLE;

//     // 2. PD 公式
//     // 直立环 = Kp * 角度偏差 + Kd * 角速度
//     // (注意：因为角速度就是角度的微分，所以这里直接乘 Gyro_Rate_DegPerSec，不需要自己去求导)
//     int PWM_Out = (int)(Vertical_Kp * Angle_Err + Vertical_Kd * Gyro_Rate_DegPerSec);

//     return PWM_Out;
// }

// // === 速度环全局参数 ===
// // 调试经验：
// // Kp: 范围通常在 50 ~ 200 之间
// // Ki: 范围通常在 Kp 的 1/200 左右 (例如 Kp=100, Ki=0.5)
// float Velocity_Kp = 0;   
// float Velocity_Ki = 0;

// // 全局变量：速度环积分累计值 (必须是全局的，因为要累加)
// float Velocity_Integral = 0; 

// // 内部变量：保存上一次的编码器读数 (用于滤波)
// int Encoder_Err_Lowout_Last = 0;

// // === 速度环 PI 控制 ===
// // 输入：Target_Speed (遥控期望速度，停止时为0), Encoder_Left/Right (左右编码器脉冲数)
// // 输出：PWM 期望值 (这个值会叠加到直立环的 PWM 上)
// // 注意：该函数建议每 50ms 或 100ms 调用一次！不要 5ms 调用！
// int Velocity_Ring(int Target_Speed, int Actual_Speed)
// {
//     int Speed_Err = Actual_Speed - Target_Speed;

//     // 低通滤波
//     // 编码器读数是跳变的，直接用会干扰直立环。我们要让速度变化“平滑”一点。
//     // 0.7 是滤波系数 (新数据占 30%，旧数据占 70%)
//     int Encoder_Err_Lowout = (int)(0.4f * Encoder_Err_Lowout_Last + 0.6f * Speed_Err);
//     Encoder_Err_Lowout_Last = Encoder_Err_Lowout; // 更新旧值

//     // 积分项计算 (I) - 只有积分才能彻底消除静止时的漂移
//     if (Encoder_Err_Lowout > 2 || Encoder_Err_Lowout < -2) {
//     Velocity_Integral += Encoder_Err_Lowout; 
//     }

//     // 积分限幅
//     // 限制在 PWM 满幅的 1/2 或 1/3 左右
//     if(Velocity_Integral > 1700)  Velocity_Integral = 1700;
//     if(Velocity_Integral < -1700) Velocity_Integral = -1700;
    
//     // 如果车倒了（PWM为0），必须清除积分，否则扶起来时车轮会狂转
//     // 这里的判断需要在外部做，或者传入一个 Stop_Flag

//     // 6. PI 公式输出
//     // 速度环 = Kp * 偏差 + Ki * 积分
//     int PWM_Out = (int)(Velocity_Kp * Encoder_Err_Lowout + Velocity_Ki * Velocity_Integral);

//     return PWM_Out;
// }

// // === 转向环全局参数 ===
// // Kp: 范围通常很小，0.5 ~ 5 之间。
// float Turn_Kp = 1.0f; // 经验值，待微调

// // === 转向环 P 控制 ===
// // 输入：Gyro_Z_Rate (Z轴真实角速度), Target_Turn_Rate (遥控期望转弯速度，直走时为0)
// // 输出：PWM 差速值 (加到左轮，减去右轮)
// int Turn_Ring(float Target_Turn_Rate, float Gyro_Z_Rate)
// {
//     // 转向的本质就是维持 Z 轴的角速度为目标值
//     float Turn_Err = Gyro_Z_Rate - Target_Turn_Rate;

//     // 只需要 P 控制即可，抗扰动
//     int PWM_Out = (int)(Turn_Kp * Turn_Err);

//     return PWM_Out;
// }


// === 速度环 PI 控制 ===
// 输入：Target_Speed (遥控期望速度，停止时为0), Encoder_Left/Right (左右编码器脉冲数)
// 输出：PWM 期望值 (这个值会叠加到直立环的 PWM 上)
// 注意：该函数建议 10ms 调用一次
float Velocity_Ring(PID_t *p, float Target_Speed, float Actual_Speed)
{
    // 1. 计算当前误差
    float current_error = Target_Speed - Actual_Speed;

    // // 2. 积分项累加 (带有死区处理)
    // // 只有当误差大于一定程度时才积分，防止原地“抽搐”
    // if (current_error > 0.005f || current_error < -0.005f) {
    //     p->integral += current_error * 0.01f; // 乘以控制周期，积分项单位是速度*秒
    // } else {
    //     p->integral *= 0.8f; // 误差极小时清空积分，防止静止漂移
    // }

    p->integral += current_error * 0.01f;
    // 只在目标速度为0时清积分，防止静止漂移
    if (Target_Speed == 0) {
        p->integral *= 0.8f;
}

    // 3. 积分限幅 (抗饱和)
    p->integral = Amplitude_Limit(p->integral, 10.0f); 
    
    // 4. PI 公式输出
    float out = (p->Kp * current_error) + (p->Ki * p->integral);

    // 5. 整体输出限幅 (对应你的定时器 ARR 值)
    int16_t PWM_Out = (int16_t)Amplitude_Limit(out, 3599.0f);

    return PWM_Out;
}

float Amplitude_Limit(float value, float max_limit)
{
    if (value > max_limit)
    {
        return max_limit;
    }
    else if (value < -max_limit)
    {
        return -max_limit;
    }
    else
    {
        return value;
    }
}


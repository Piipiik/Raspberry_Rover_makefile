#include "kinematics.h"
#include "stm32f1xx_hal_tim.h"
#include <string.h>

const float PI = 3.14159265f;
const float WHEEL_DIAMETER_MM = 3.0f * 25.4f; // 3英寸转mm
const float ENCODER_TICKS_PER_REV = 1320.0f;  // 11*4*300.0f; // 11齿盘 * 4倍频 * 30减速比
const float WHEEL_BASE = 0.21f; // 前后轴距，单位mm
const float TRACK_WIDTH = 0.2f; // 左右轮距，单位mm

const float CONTROL_PERIOD_S = 0.01f;


// 初始化：清空所有数据
void Kinematics_Init(Kinematics_t *kin) {
    memset(kin, 0, sizeof(Kinematics_t));

    Kinematics_SetWheelDistance(kin, WHEEL_BASE, TRACK_WIDTH); // 使用全局变量

    // 计算系数
    float per_pulse_dist = (PI * WHEEL_DIAMETER_MM) / ENCODER_TICKS_PER_REV; 
    // 结果大约是 0.181355mm/脉冲
    for(int i = 0; i < MOTOR_COUNT; i++) {
        // 填入计算出的系数
        Kinematics_SetMotorParam(kin, (MotorID_t)i, per_pulse_dist); 
    }
}

// 设置轮距与轴距：计算出那个关键的 l_sum
void Kinematics_SetWheelDistance(Kinematics_t *kin, float wheel_base, float track_width) {
    kin->wheel_base = wheel_base;
    kin->track_width = track_width;
    // 麦轮物理特性的几何常数：长宽之和的一半
    kin->l_sum = (wheel_base + track_width) / 2.0f;
}

// 设置电机系数
void Kinematics_SetMotorParam(Kinematics_t *kin, MotorID_t id, float per_pulse_distance) {
    if (id < MOTOR_COUNT) {
        kin->motor[id].per_pulse_distance = per_pulse_distance;
    }
}

/**
 * @brief 麦克纳姆轮逆解：车体速度 -> 4个轮子的目标线速度
 * @param vx 期望前后速度 (前进为正, mm/s)
 * @param vy 期望左右速度 (左移为正, mm/s)
 * @param wz 期望旋转角速度 (逆时针为正, rad/s)
 * @param speeds 输出数组，存储 LF, RF, LR, RR 的目标线速度
 */
void Kinematics_Inverse(Kinematics_t *kin, float vx, float vy, float wz, float *speeds) {
    // 1. 左前轮 (Left Front)
    speeds[0] = vx - vy - wz * kin->l_sum;
    // 2. 右前轮 (Right Front)
    speeds[1] = vx + vy + wz * kin->l_sum;
    // 3. 左后轮 (Left Rear)
    speeds[2] = vx + vy - wz * kin->l_sum;
    // 4. 右后轮 (Right Rear)
    speeds[3] = vx - vy + wz * kin->l_sum;
}

/**
 * @brief 麦克纳姆轮正解：4个轮子的实际线速度 -> 车体实际速度
 * @param speeds 输入数组，当前 LF, RF, LR, RR 的实际线速度 (mm/s)
 * @param vx 输出当前前后方向实际速度
 * @param vy 输出当前左右方向实际速度
 * @param wz 输出当前自转实际角速度
 */
void Kinematics_Forward(Kinematics_t *kin, float *vx, float *vy, float *wz) {
    // 正解是逆解的矩阵求逆运算（或者简单的算术平均）
    *vx = (kin->motor[MOTOR_LF].current_speed + kin->motor[MOTOR_RF].current_speed + kin->motor[MOTOR_LR].current_speed + kin->motor[MOTOR_RR].current_speed) / 4.0f;
    *vy = (-kin->motor[MOTOR_LF].current_speed + kin->motor[MOTOR_RF].current_speed + kin->motor[MOTOR_LR].current_speed - kin->motor[MOTOR_RR].current_speed) / 4.0f;
    *wz = (-kin->motor[MOTOR_LF].current_speed + kin->motor[MOTOR_RF].current_speed - kin->motor[MOTOR_LR].current_speed + kin->motor[MOTOR_RR].current_speed) / (4.0f * kin->l_sum);
}

/**
 * @brief 更新电机速度
 * @param ticks 传入一个包含4个电机当前计数值的数组
 */
void Kinematics_UpdateMotorSpeed(Kinematics_t *kin, Motor_t *Motor) {
    for (int id = 0; id < MOTOR_COUNT; id++) {
            // 1. 读取当前定时器的编码器计数值
            int16_t current_ticks = (int16_t)__HAL_TIM_GET_COUNTER(Motor[id].enc_tim);

            // 2. 计算脉冲差值
            // (强制转换为 int16_t 可以完美且自动地处理 65535->0 或 0->65535 的溢出回环)
            int16_t delta_tick = (int16_t)(current_ticks - kin->motor[id].last_tick);

            // __HAL_TIM_SET_COUNTER(Motor[id].enc_tim, 0);

            // 3. 计算这一拍走过的物理距离 (单位：米 m)
            // 公式：脉冲差 * 毫米/脉冲系数 * 电机极性(1或-1) / 1000.0f
            float distance = delta_tick * kin->motor[id].per_pulse_distance * Motor[id].polarity / 1000.0f;

            // 5. 计算最终的线速度 (单位：m/s)
            // 公式：速度 = 距离 / 时间
            kin->motor[id].current_speed = distance / CONTROL_PERIOD_S;

            // 6. 保存当前计数值，供下一次 (10ms后) 使用
            kin->motor[id].last_tick = current_ticks;
        }
}

int16_t Kinematics_GetMotorSpeed(Kinematics_t *kin, MotorID_t id) {
    if (id < MOTOR_COUNT) {
        return (int16_t)kin->motor[id].current_speed;
    }
    return 0;
}
#include "filter.h"
#include <math.h> // 必须包含数学库
#include "mpu6050.h"
#include <main.h>


// 互补滤波参数
#define K_FILTER  0.98f   // 权重系数 (0.95 ~ 0.98)
#define DT        0.005f  // 采样时间 (秒)，假设你每 10ms 调用一次

// 全局变量，用来保存当前的最终角度
// 必须是全局的，因为它是“积分”的，要记得上一次的值
float Pitch_Angle = 0; 

// 我测着是-39
float Gyro_X_Offset = -39; // 全局变量，存偏差值
float Gyro_Z_Offset = -29;

void Filter_Init(void)
{
    int16_t ax, ay, az, gx, gy, gz;

    // 给用户一个心理准备时间，也等待传感器内部稳定
    HAL_Delay(500); 

    // === 核心：快速收敛循环 ===
    // 循环 100 次，每次间隔 5ms (模拟真实的控制周期)
    // 目的：利用互补滤波的特性，让 Pitch_Angle 快速追上真实的重力角度
    for(int i = 0; i < 100; i++)
    {
        // 1. 获取数据
        MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);

        // 2. 跑一遍姿态解算
        // 【重要】：这里一定要填你刚才测试通过的、正确的轴！！！
        // 如果你测出来是 ax 和 gx，就填 ax, az, gx
        // 如果还是 ay 和 gy，就填 ay, az, gy
        MPU6050_Update_Posture(ay, az, gx); 
        
        // 3. 延时 5ms，模拟真实采样率
        HAL_Delay(5);
    }
    
    // 循环结束后，Pitch_Angle 已经等于当前真实的物理角度了。
    // 此时再启动电机，如果角度 > 45，电机绝对不会转。
}

// 新增：校准函数
void MPU6050_Calibrate_Gyro(void)
{
    int32_t Sum = 0;
    int16_t ax, ay, az, gx, gy, gz;

    // 丢弃前几次数据：MPU6050 刚上电或复位时，前几组数据可能会波动较大。
     for(int i=0; i<100; i++)
    {
        MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
        HAL_Delay(5);
    }
    
    // 循环读 200 次，耗时约 1 秒
    for(int i=0; i<200; i++)
    {
        MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
        Sum += gx; // 累加 GX 数据
        HAL_Delay(5);
    }
    
    // 求平均值
    Gyro_X_Offset = (float)Sum / 200.0f;
}

// 新增：校准函数
void MPU6050_Calibrate_Gyro_gz(void)
{
    int32_t Sum = 0;
    int16_t ax, ay, az, gx, gy, gz;

    // 丢弃前几次数据：MPU6050 刚上电或复位时，前几组数据可能会波动较大。
     for(int i=0; i<100; i++)
    {
        MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
        HAL_Delay(5);
    }
    
    // 循环读 200 次，耗时约 1 秒
    for(int i=0; i<200; i++)
    {
        MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
        Sum += gz; // 累加 GX 数据
        HAL_Delay(5);
    }
    
    // 求平均值
    Gyro_Z_Offset = (float)Sum / 200.0f;
}

// 计算加速度计的倾角 (结果是度)
// 参数：ay, az 是 MPU6050 读出来的原始数据 (int16_t)
float Get_Accel_Angle(int16_t ay, int16_t az) 
{
    // 核心公式：angle = atan2(y, z)
    // 乘以 57.296 是为了把 弧度 变成 角度 (180 / PI)
    // 注意：这里可能需要根据你的安装方向调整，可能是 atan2(ay, az)
    float angle = atan2(ay, az) * 57.296f; 
    
    return angle;
}

// 计算陀螺仪角速度 (结果是 度/秒)
// 参数：gx 是 MPU6050 读出来的原始数据
float Get_Gyro_Rate(int16_t gx) 
{
    // 量程 ±2000 对应的灵敏度是 16.4 (32767/2000)
    return (gx - Gyro_X_Offset) / 16.4f;
}

float Get_Gyro_Rate_gz(int16_t gz) 
{
    // 量程 ±2000 对应的灵敏度是 16.4 (32767/2000)
    return (gz - Gyro_Z_Offset) / 16.4f;
}

// 姿态解算函数
void MPU6050_Update_Posture(int16_t ay, int16_t az, int16_t gx)
{
    // 1. 获取 两个“不完美”的输入
    float Accel_Angle = Get_Accel_Angle(ay, az); // "静态准"的角度
    float Gyro_Rate   = Get_Gyro_Rate(gx);       // "动态准"的速度

    // 2. 执行互补滤波公式
    // Pitch = 0.95 * (上次角度 + 角速度积分) + 0.05 * (加速度角度)
    Pitch_Angle = K_FILTER * (Pitch_Angle + Gyro_Rate * DT) + (1 - K_FILTER) * Accel_Angle;
}

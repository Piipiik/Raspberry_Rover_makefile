#include "mpu6050.h"
#include "mpu6050_reg.h" // 刚才生成的那个寄存器头文件
#include "bsp_soft_i2c.h"       // 假设你已经有了 MyI2C_SendByte 等函数
#include <main.h>

// 核心：定义一个内部指针，用来记录当前 OLED 使用的是哪条 I2C 总线
static SoftI2C_Type *g_mpu6050_i2c = NULL;

// 辅助函数：写一个字节到指定寄存器
// 如果你还没封装这个，可以直接抄
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
    MyI2C_Start(g_mpu6050_i2c);
    MyI2C_SendByte(g_mpu6050_i2c, MPU6050_ADDR_AD0_LOW); // 发送设备地址+写 (0xD0)
    MyI2C_WaitAck(g_mpu6050_i2c);
    MyI2C_SendByte(g_mpu6050_i2c, RegAddress);           // 发送寄存器地址
    MyI2C_WaitAck(g_mpu6050_i2c);
    MyI2C_SendByte(g_mpu6050_i2c, Data);                 // 发送数据
    MyI2C_WaitAck(g_mpu6050_i2c);
    MyI2C_Stop(g_mpu6050_i2c);
}

uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
    uint8_t Data;
    
    MyI2C_Start(g_mpu6050_i2c);
    MyI2C_SendByte(g_mpu6050_i2c, MPU6050_ADDR_AD0_LOW); 
    MyI2C_WaitAck(g_mpu6050_i2c);
    MyI2C_SendByte(g_mpu6050_i2c, RegAddress);
    MyI2C_WaitAck(g_mpu6050_i2c);
    
    MyI2C_Start(g_mpu6050_i2c);
    MyI2C_SendByte(g_mpu6050_i2c, (MPU6050_ADDR_AD0_LOW) | 0x01); 
    MyI2C_WaitAck(g_mpu6050_i2c);
    Data = MyI2C_ReceiveByte(g_mpu6050_i2c);
    MyI2C_SendAck(g_mpu6050_i2c, 1); // 发送 NACK (结束读取)
    MyI2C_Stop(g_mpu6050_i2c);
    
    return Data;
}

// MPU6050 初始化函数
void MPU6050_Init(SoftI2C_Type *i2c_bus)
{
    g_mpu6050_i2c = i2c_bus;

    // 0. 初始化 I2C 底层 (SCL/SDA GPIO)
    MyI2C_Init(g_mpu6050_i2c); 

    // 1. 解除睡眠模式 (必须！)
    // 寄存器: PWR_MGMT_1 (0x6B)
    // 写入 0x00: 唤醒，使用内部 8MHz 时钟
    // (如果写 0x01 则使用 X轴陀螺仪作为时钟源，更稳一点，推荐 0x01)
    MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);
    // HAL_Delay(100);

    // 2. 设置采样率分频
    // 陀螺仪内部的“输出频率”固定为 1000Hz（即 1ms 产出一个数据）
    // 寄存器: SMPLRT_DIV (0x19)
    // 采样率 = 陀螺仪输出率 / (1 + SMPLRT_DIV)
    // 写入 0x00: 不分频，采样率最快 (1kHz 或 8kHz)
    MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x00);

    // 3. 配置低通滤波 (DLPF)
    // 寄存器: CONFIG (0x1A)
    // 写入 0x06: 5Hz 带宽 (滤波最强，数据最平滑，延迟约19ms)
    // 写入 0x00: 260Hz 带宽 (几乎无滤波，反应最快，也就是最抖)
    // 建议: 平衡车用 0x02 (94Hz，延迟约3ms)  或 0x03 (44Hz，延迟约4.9ms) 比较合适
    MPU6050_WriteReg(MPU6050_CONFIG, 0x03); 

    // 4. 配置陀螺仪量程
    // 寄存器: GYRO_CONFIG (0x1B)
    // Bit[4:3] FS_SEL:
    // 0x00(±250dps), 0x08(±500), 0x10(±1000), 0x18(±2000)
    // 建议: ±2000dps (对应寄存器写 0x18)，防止快速旋转时爆表
    MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);

    // 5. 配置加速度计量程
    // 寄存器: ACCEL_CONFIG (0x1C)
    // Bit[4:3] AFS_SEL:
    // 0x00(±2g), 0x08(±4g), 0x10(±8g), 0x18(±16g)
    // 建议: ±2g (0x00) 或 ±4g (0x08)。平衡车一般不会超过 2g 加速度。
    MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x00);
}

// 获取设备 ID (用于测试通信是否正常)
// 返回值应该是 0x68
uint8_t MPU6050_GetID(void)
{
    uint8_t id;
    MyI2C_Start(g_mpu6050_i2c);
    MyI2C_SendByte(g_mpu6050_i2c, MPU6050_ADDR_AD0_LOW); // 写地址
    MyI2C_WaitAck(g_mpu6050_i2c);
    MyI2C_SendByte(g_mpu6050_i2c, MPU6050_WHO_AM_I);     // 寄存器 WHO_AM_I (0x75)
    MyI2C_WaitAck(g_mpu6050_i2c);
    
    MyI2C_Start(g_mpu6050_i2c); // 重复起始信号 (Restart)
    MyI2C_SendByte(g_mpu6050_i2c, MPU6050_ADDR_AD0_LOW | 0x01); // 读地址 (0xD1)
    MyI2C_WaitAck(g_mpu6050_i2c);
    id = MyI2C_ReceiveByte(g_mpu6050_i2c);             // 接收 ID
    MyI2C_SendAck(g_mpu6050_i2c, 1);                     // 发送 NACK (告诉它我不读了)
    MyI2C_Stop(g_mpu6050_i2c);
    
    return id;
}

// 参数是6个指针，用来接收读取到的 6 个轴的数据
// I2C 连续读取函数
// RegAddress: 起始寄存器地址
// Buffer: 存放读到数据的数组指针
// Count: 要读多少个字节
void MPU6050_ReadBlock(uint8_t RegAddress, uint8_t *Buffer, uint16_t Count)
{
    MyI2C_Start(g_mpu6050_i2c);
    // 1. 发送写地址 (0xD0)
    MyI2C_SendByte(g_mpu6050_i2c, MPU6050_ADDR_AD0_LOW); 
    MyI2C_WaitAck(g_mpu6050_i2c);
    
    // 2. 发送起始寄存器地址
    MyI2C_SendByte(g_mpu6050_i2c, RegAddress); 
    MyI2C_WaitAck(g_mpu6050_i2c);
    
    // 3. 重复起始信号，准备开始读
    MyI2C_Start(g_mpu6050_i2c);
    // 4. 发送读地址 (0xD1)
    MyI2C_SendByte(g_mpu6050_i2c, (MPU6050_ADDR_AD0_LOW) | 0x01); 
    MyI2C_WaitAck(g_mpu6050_i2c);
    
    // 5. 循环接收数据
    for (uint16_t i = 0; i < Count; i++) {
        // 接收一个字节
        Buffer[i] = MyI2C_ReceiveByte(g_mpu6050_i2c);
        
        // 关键点：应答机制
        if (i < Count - 1) {
            MyI2C_SendAck(g_mpu6050_i2c, 0); // 如果还没读完，给应答(ACK)，告诉从机“继续发”
        } else {
            MyI2C_SendAck(g_mpu6050_i2c, 1); // 如果读完了，给非应答(NACK)，告诉从机“别发了”
        }
    }
    
    MyI2C_Stop(g_mpu6050_i2c);
}

void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
                     int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
    uint8_t Buffer[14]; // 创建一个缓存数组
    
    // 从 ACCEL_XOUT_H (0x3B) 开始，连续读 14 个字节
    // 顺序是: AccX_H, AccX_L, AccY_H, AccY_L, AccZ_H, AccZ_L, 
    //        Temp_H, Temp_L (温度), 
    //        GyroX_H, GyroX_L ...
    MPU6050_ReadBlock(MPU6050_ACCEL_XOUT_H, Buffer, 14);
    
    // 开始拼接数据 (高位左移8位 | 低位)
    *AccX  = (int16_t)(Buffer[0] << 8 | Buffer[1]);
    *AccY  = (int16_t)(Buffer[2] << 8 | Buffer[3]);
    *AccZ  = (int16_t)(Buffer[4] << 8 | Buffer[5]);
    
    // Buffer[6] 和 Buffer[7] 是温度数据，这里我们跳过它
    // 如果你以后想读温度，就是 temp = Buffer[6]<<8 | Buffer[7];
    
    *GyroX = (int16_t)(Buffer[8] << 8 | Buffer[9]);
    *GyroY = (int16_t)(Buffer[10] << 8 | Buffer[11]);
    *GyroZ = (int16_t)(Buffer[12] << 8 | Buffer[13]);
}

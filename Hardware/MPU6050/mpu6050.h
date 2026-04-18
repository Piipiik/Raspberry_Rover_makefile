#ifndef  __MPU6050__H
#define  __MPU6050__H

#include "bsp_soft_i2c.h" // [移植说明] 这里换成你的I2C驱动头文件
#include <stdint.h>

void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data);
uint8_t MPU6050_ReadReg(uint8_t RegAddress);

// MPU6050 初始化函数
void MPU6050_Init(SoftI2C_Type *i2c_bus);

// 获取设备 ID (用于测试通信是否正常)
// 返回值是 0x68
uint8_t MPU6050_GetID(void);

// 参数是6个指针，用来接收读取到的 6 个轴的数据
void MPU6050_ReadBlock(uint8_t RegAddress, uint8_t *Buffer, uint16_t Count);


void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
                     int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ);



#endif
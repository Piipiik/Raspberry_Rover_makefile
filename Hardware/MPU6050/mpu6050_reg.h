#ifndef __MPU6050_REG_H__
#define __MPU6050_REG_H__

/*
 * MPU6050 Register Map
 * Source: RM-MPU-6000A-00 Revision 3.2
 */

// ==============================================================================
// 1. I2C 从机地址 (Slave Address)
// ==============================================================================
// 取决于 AD0 引脚的电平状态 [cite: 1136, 1138]
// 绝大多数模块 AD0 悬空或接地，使用 LOW 地址
#define MPU6050_ADDR_AD0_LOW     0xD0 
#define MPU6050_ADDR_AD0_HIGH    0xD1

// ==============================================================================
// 2. 寄存器地址定义 (Register Addresses)
// ==============================================================================

// --- 辅助 I2C 电源配置 ---
#define MPU6050_AUX_VDDIO        0x01 // [cite: 99]

// --- 采样率与滤波配置 ---
#define MPU6050_SMPLRT_DIV       0x19 // [cite: 121]
#define MPU6050_CONFIG           0x1A // [cite: 143] (DLPF配置在这里)
#define MPU6050_GYRO_CONFIG      0x1B // [cite: 167] (量程 FS_SEL 在这里)
#define MPU6050_ACCEL_CONFIG     0x1C // [cite: 205] (量程 AFS_SEL 在这里)

// --- 运动检测阈值 (Motion Detection) ---
#define MPU6050_FF_THR           0x1D // [cite: 253] (自由落体阈值)
#define MPU6050_FF_DUR           0x1E // [cite: 267]
#define MPU6050_MOT_THR          0x1F // [cite: 292] (运动检测阈值)
#define MPU6050_MOT_DUR          0x20 // [cite: 307]
#define MPU6050_ZRMOT_THR        0x21 // [cite: 326] (零运动阈值)
#define MPU6050_ZRMOT_DUR        0x22 // [cite: 341]

// --- FIFO 使能 ---
#define MPU6050_FIFO_EN          0x23 // [cite: 360]

// --- I2C 主机控制 (AUX I2C) ---
#define MPU6050_I2C_MST_CTRL     0x24 // [cite: 384]

// --- I2C 从机 0~4 配置 (用于读取磁力计等外设) ---
#define MPU6050_I2C_SLV0_ADDR    0x25 // [cite: 435]
#define MPU6050_I2C_SLV0_REG     0x26 // [cite: 435]
#define MPU6050_I2C_SLV0_CTRL    0x27 // [cite: 435]
#define MPU6050_I2C_SLV1_ADDR    0x28 // [cite: 522]
#define MPU6050_I2C_SLV1_REG     0x29 // [cite: 522]
#define MPU6050_I2C_SLV1_CTRL    0x2A // [cite: 522]
#define MPU6050_I2C_SLV2_ADDR    0x2B // [cite: 530]
#define MPU6050_I2C_SLV2_REG     0x2C // [cite: 530]
#define MPU6050_I2C_SLV2_CTRL    0x2D // [cite: 530]
#define MPU6050_I2C_SLV3_ADDR    0x2E // [cite: 536]
#define MPU6050_I2C_SLV3_REG     0x2F // [cite: 536]
#define MPU6050_I2C_SLV3_CTRL    0x30 // [cite: 536]
#define MPU6050_I2C_SLV4_ADDR    0x31 // [cite: 549]
#define MPU6050_I2C_SLV4_REG     0x32 // [cite: 549]
#define MPU6050_I2C_SLV4_DO      0x33 // [cite: 549]
#define MPU6050_I2C_SLV4_CTRL    0x34 // [cite: 549]
#define MPU6050_I2C_SLV4_DI      0x35 // [cite: 549]

// --- 状态寄存器 ---
#define MPU6050_I2C_MST_STATUS   0x36 // [cite: 590]
#define MPU6050_INT_PIN_CFG      0x37 // [cite: 606] (旁路模式 BYPASS在这里开)
#define MPU6050_INT_ENABLE       0x38 // [cite: 650] (数据就绪中断在这里开)
#define MPU6050_INT_STATUS       0x3A // [cite: 668] (读这个清中断)

// --- 传感器原始数据 (只读, High字节在前) ---
// 加速度计 (Accelerometer)
#define MPU6050_ACCEL_XOUT_H     0x3B // [cite: 688]
#define MPU6050_ACCEL_XOUT_L     0x3C // [cite: 688]
#define MPU6050_ACCEL_YOUT_H     0x3D // [cite: 688]
#define MPU6050_ACCEL_YOUT_L     0x3E // [cite: 688]
#define MPU6050_ACCEL_ZOUT_H     0x3F // [cite: 688]
#define MPU6050_ACCEL_ZOUT_L     0x40 // [cite: 688]

// 温度传感器 (Temperature)
#define MPU6050_TEMP_OUT_H       0x41 // [cite: 719]
#define MPU6050_TEMP_OUT_L       0x42 // [cite: 719]

// 陀螺仪 (Gyroscope)
#define MPU6050_GYRO_XOUT_H      0x43 // [cite: 744]
#define MPU6050_GYRO_XOUT_L      0x44 // [cite: 744]
#define MPU6050_GYRO_YOUT_H      0x45 // [cite: 744]
#define MPU6050_GYRO_YOUT_L      0x46 // [cite: 744]
#define MPU6050_GYRO_ZOUT_H      0x47 // [cite: 744]
#define MPU6050_GYRO_ZOUT_L      0x48 // [cite: 744]

// --- 外设数据 (External Sensor Data) ---
#define MPU6050_EXT_SENS_DATA_00 0x49 // [cite: 772]
// ... (中间省略连续的寄存器，使用时通常用基地址+偏移)
#define MPU6050_EXT_SENS_DATA_23 0x60 // [cite: 772]

// --- 运动检测状态 ---
#define MPU6050_MOT_DETECT_STATUS 0x61 // [cite: 818]

// --- I2C 从机数据输出 ---
#define MPU6050_I2C_SLV0_DO      0x63 // [cite: 838]
#define MPU6050_I2C_SLV1_DO      0x64 // [cite: 849]
#define MPU6050_I2C_SLV2_DO      0x65 // [cite: 865]
#define MPU6050_I2C_SLV3_DO      0x66 // [cite: 876]

// --- 延迟控制与重置 ---
#define MPU6050_I2C_MST_DELAY_CTRL 0x67 // [cite: 893]
#define MPU6050_SIGNAL_PATH_RESET 0x68 // [cite: 922]
#define MPU6050_MOT_DETECT_CTRL  0x69 // [cite: 942]

// --- 核心控制寄存器 ---
#define MPU6050_USER_CTRL        0x6A // [cite: 974] (I2C主模式使能)
#define MPU6050_PWR_MGMT_1       0x6B // [cite: 1022] (复位、时钟源选择、睡眠模式)
#define MPU6050_PWR_MGMT_2       0x6C // [cite: 1064] (循环模式唤醒频率)

// --- FIFO 计数与数据 ---
#define MPU6050_FIFO_COUNT_H     0x72 // [cite: 1087]
#define MPU6050_FIFO_COUNT_L     0x73 // [cite: 1087]
#define MPU6050_FIFO_R_W         0x74 // [cite: 1108]

// --- 器件 ID ---
#define MPU6050_WHO_AM_I         0x75 // [cite: 1132] (默认值应为 0x68)


#endif // __MPU6050_REG_H__
#ifndef BSP_SOFT_I2C_H
#define BSP_SOFT_I2C_H

/* 如果是 STM32 HAL 库，包含 main.h 即可获取所有 HAL 定义 */
/* 如果是标准库或寄存器开发，包含对应的 stm32f10x.h 等 */
#include "main.h" 
#include <stdint.h>

/* ==================================================
 * I2C 软件驱动接口声明
 * 注意：硬件引脚配置请去 .c 文件中修改
 * ================================================== */

typedef struct {
    GPIO_TypeDef* port;    // GPIO 端口 (如 GPIOC)
    uint16_t scl_pin;      // SCL 引脚
    uint16_t sda_pin;      // SDA 引脚
} SoftI2C_Type;

// 总线初始化 (置空闲状态)
void MyI2C_Init(SoftI2C_Type* i2c);

// 基础信号控制
void MyI2C_Start(SoftI2C_Type* i2c);
void MyI2C_Stop(SoftI2C_Type* i2c);

// 字节收发
void MyI2C_SendByte(SoftI2C_Type* i2c, uint8_t Byte);
uint8_t MyI2C_ReceiveByte(SoftI2C_Type* i2c);

// 应答信号控制
void MyI2C_SendAck(SoftI2C_Type* i2c, uint8_t AckBit); // 0:ACK (继续), 1:NACK (停止)
uint8_t MyI2C_WaitAck(SoftI2C_Type* i2c);        // 返回 0:ACK, 1:NACK

void I2C_Fix_Deadlock(SoftI2C_Type* i2c);        //防止总线死锁

#endif
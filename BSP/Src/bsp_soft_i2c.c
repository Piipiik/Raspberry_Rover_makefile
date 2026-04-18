#include "bsp_soft_i2c.h"

/* ==============================================================================
 * 【移植适配区】 修改此处以适配不同的 GPIO 和 主频
 * ============================================================================== */

// 1. 定义操作的端口和引脚 (根据 CubeMX/原理图 修改)

// #define I2C_PORT        GPIOC
// #define I2C_SCL_PIN     GPIO_PIN_10
// #define I2C_SDA_PIN     GPIO_PIN_11

// 2. 软件延时参数
// 经验值：72MHz主频下，30 大约对应 100~200kHz 速率，非常稳健
// 如果主频更高 (如 F4/H7)，请按比例增大此值
#define I2C_DELAY_CNT   100

// // 3. 底层引脚操作宏 (封装 HAL 库函数)

// // 写引脚电平
// #define I2C_W_SCL(x)    HAL_GPIO_WritePin(I2C_PORT, I2C_SCL_PIN, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
// #define I2C_W_SDA(x)    HAL_GPIO_WritePin(I2C_PORT, I2C_SDA_PIN, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
// // 读引脚电平
// #define I2C_R_SDA()     HAL_GPIO_ReadPin(I2C_PORT, I2C_SDA_PIN)

// --- 替换为寄存器操作 (速度提升 5-10 倍) ---
// // BSRR 低 16 位是置 1 (High)，高 16 位是置 0 (Low)
// #define I2C_W_SCL(x)    ((x) ? (I2C_PORT->BSRR = I2C_SCL_PIN) : (I2C_PORT->BSRR = (uint32_t)I2C_SCL_PIN << 16))
// #define I2C_W_SDA(x)    ((x) ? (I2C_PORT->BSRR = I2C_SDA_PIN) : (I2C_PORT->BSRR = (uint32_t)I2C_SDA_PIN << 16))
// // 读引脚 IDR
// #define I2C_R_SDA()     ((GPIOB->IDR & GPIO_PIN_11) != 0)

static void I2C_W_SCL(SoftI2C_Type* i2c, uint8_t x) {
    if(x) i2c->port->BSRR = i2c->scl_pin;
    else  i2c->port->BSRR = (uint32_t)i2c->scl_pin << 16;
}

static void I2C_W_SDA(SoftI2C_Type* i2c, uint8_t x) {
    if(x) i2c->port->BSRR = i2c->sda_pin;
    else  i2c->port->BSRR = (uint32_t)i2c->sda_pin << 16;
}

static uint8_t I2C_R_SDA(SoftI2C_Type* i2c) {
    return (i2c->port->IDR & i2c->sda_pin) != 0;
}

/* ==============================================================================
 * 【通用逻辑区】 以下代码通常不需要修改
 * ============================================================================== */

// #define MyI2C_Delay()

/**
  * @brief  微秒级软件延时 (非精确，仅用于控制时序间隔)
  */
static void MyI2C_Delay(void)
{
    volatile uint32_t i = I2C_DELAY_CNT;
    while (i--);
}


/**
  * @brief  I2C总线初始化
  * @note   注意：GPIO的模式配置(开漏输出/上拉)应在 main.c 或 HAL_GPIO_Init 中完成
  */
void MyI2C_Init(SoftI2C_Type* i2c)
{
    // 释放总线，确保 SCL 和 SDA 均为高电平 (空闲状态)
    I2C_W_SCL(i2c, 1);
    I2C_W_SDA(i2c, 1);
}

/**
  * @brief  产生起始信号 (Start)
  * @note   SCL高电平期间，SDA由高变低
  */
void MyI2C_Start(SoftI2C_Type* i2c)
{
    I2C_W_SDA(i2c, 1);
    I2C_W_SCL(i2c, 1);

    I2C_W_SDA(i2c, 0);
    MyI2C_Delay(); // 【协议强制】Start 保持时间 (t_HD;STA)
    
    I2C_W_SCL(i2c, 0);  // 钳住总线，准备发送数据
}

/**
  * @brief  产生停止信号 (Stop)
  * @note   SCL高电平期间，SDA由低变高
  */
void MyI2C_Stop(SoftI2C_Type* i2c)
{
    I2C_W_SDA(i2c, 0);
    MyI2C_Delay(); // 确保 SDA 已经稳定为低
    
    I2C_W_SCL(i2c, 1);
    MyI2C_Delay(); // 【协议强制】Stop 建立时间 (t_SU;STO)
    
    I2C_W_SDA(i2c, 1);
}

// void MyI2C_SendByte(SoftI2C_Type* i2c, uint8_t Byte)
// {
//     uint8_t i;
//     for (i = 0; i < 8; i++)
//     {
//         // 1. 准备数据 (SCL 低电平期间)
//         if (Byte & (0x80 >> i)) {
//             I2C_W_SDA(i2c, 1);
//         } else {
//             I2C_W_SDA(i2c, 0);
//         }
//         MyI2C_Delay(); // 数据建立时间
        
//         // 2. 拉高时钟，让从机读取
//         I2C_W_SCL(i2c, 1);
//         MyI2C_Delay(); // 时钟高电平时间
        
//         // 3. 拉低时钟，准备下一位
//         I2C_W_SCL(i2c, 0);
//     }
// }

/**
  * @brief  发送一个字节 (MSB First)
  */
void MyI2C_SendByte(SoftI2C_Type* i2c, uint8_t Byte)
{
    // ---------------- 第7位 (最高位) ----------------
    // 准备数据
    I2C_W_SCL(i2c, 0); 
    if(Byte & 0x80) I2C_W_SDA(i2c, 1); else I2C_W_SDA(i2c, 0);
    MyI2C_Delay();
    I2C_W_SCL(i2c, 1);
    MyI2C_Delay();
    
    // ---------------- 第6位 ----------------
    I2C_W_SCL(i2c, 0); 
    if(Byte & 0x40) I2C_W_SDA(i2c, 1); else I2C_W_SDA(i2c, 0);
    MyI2C_Delay();
    I2C_W_SCL(i2c, 1);
    MyI2C_Delay();

    // ---------------- 第5位 ----------------
    I2C_W_SCL(i2c, 0); 
    if(Byte & 0x20) I2C_W_SDA(i2c, 1); else I2C_W_SDA(i2c, 0);
    MyI2C_Delay();
    I2C_W_SCL(i2c, 1);
    MyI2C_Delay();

    // ---------------- 第4位 ----------------
    I2C_W_SCL(i2c, 0); 
    if(Byte & 0x10) I2C_W_SDA(i2c, 1); else I2C_W_SDA(i2c, 0);
    MyI2C_Delay();
    I2C_W_SCL(i2c, 1);
    MyI2C_Delay();

    // ---------------- 第3位 ----------------
    I2C_W_SCL(i2c, 0); 
    if(Byte & 0x08) I2C_W_SDA(i2c, 1); else I2C_W_SDA(i2c, 0);
    MyI2C_Delay();
    I2C_W_SCL(i2c, 1);
    MyI2C_Delay();

    // ---------------- 第2位 ----------------
    I2C_W_SCL(i2c, 0); 
    if(Byte & 0x04) I2C_W_SDA(i2c, 1); else I2C_W_SDA(i2c, 0);
    MyI2C_Delay();
    I2C_W_SCL(i2c, 1);
    MyI2C_Delay();

    // ---------------- 第1位 ----------------
    I2C_W_SCL(i2c, 0); 
    if(Byte & 0x02) I2C_W_SDA(i2c, 1); else I2C_W_SDA(i2c, 0);
    MyI2C_Delay();
    I2C_W_SCL(i2c, 1);
    MyI2C_Delay();

    // ---------------- 第0位 ----------------
    I2C_W_SCL(i2c, 0); 
    if(Byte & 0x01) I2C_W_SDA(i2c, 1); else I2C_W_SDA(i2c, 0);
    MyI2C_Delay();
    I2C_W_SCL(i2c, 1);
    MyI2C_Delay();

    // 最后拉低SCL，结束
    I2C_W_SCL(i2c, 0);
}

/**
  * @brief  接收一个字节
  * @note   主机在读取前必须释放 SDA 线
  */
uint8_t MyI2C_ReceiveByte(SoftI2C_Type* i2c)
{
    uint8_t i;
    uint8_t Byte = 0;
    
    // 主机释放 SDA (切换为输入，开漏输出写1即释放)
    I2C_W_SDA(i2c, 1); 
    MyI2C_Delay();
    
    for (i = 0; i < 8; i++)
    {
        I2C_W_SCL(i2c, 1);       // 拉高时钟，从机数据有效
        MyI2C_Delay();
        
        if (I2C_R_SDA(i2c)) {
            Byte |= (0x80 >> i);
        }
        
        I2C_W_SCL(i2c, 0);       // 拉低时钟
        MyI2C_Delay();
    }
    return Byte;
}

/**
  * @brief  发送应答信号
  * @param  AckBit 0:ACK (继续), 1:NACK (停止)
  */
void MyI2C_SendAck(SoftI2C_Type* i2c, uint8_t AckBit)
{
    if (AckBit) {
        I2C_W_SDA(i2c, 1);
    } else {
        I2C_W_SDA(i2c, 0);
    }
    MyI2C_Delay();
    
    I2C_W_SCL(i2c, 1);
    MyI2C_Delay();
    
    I2C_W_SCL(i2c, 0);
}

/**
  * @brief  等待从机应答
  * @return 0:接收到ACK, 1:接收到NACK
  */
uint8_t MyI2C_WaitAck(SoftI2C_Type* i2c)
{
    uint8_t AckBit;
    
    I2C_W_SDA(i2c, 1);   // 释放总线，交由从机控制
    MyI2C_Delay();
    
    I2C_W_SCL(i2c, 1);
    MyI2C_Delay();  // 给从机拉低的时间
    
    if (I2C_R_SDA(i2c)) {
        AckBit = 1; // 读到高电平，说明从机没拉低 (NACK)
    } else {
        AckBit = 0; // 读到低电平，说明从机拉低了 (ACK)
    }
    
    I2C_W_SCL(i2c, 0);
    
    return AckBit;
}

// // 极速版 WaitAck (不检测应答，不切换输入输出模式)
// // 注意：这要求你的 SDA 引脚一直是 开漏输出 (Output Open-Drain) 模式！
// uint8_t MyI2C_WaitAck(SoftI2C_Type* i2c)
// {
//     I2C_W_SDA(i2c, 1); // 释放 SDA (开漏输出写1就是释放，不需要切输入模式)
    
//     I2C_W_SCL(i2c, 1); // 产生第9个脉冲，让 OLED 拉低 SDA (虽然我们不看)
//     // MyI2C_Delay(); 
    
//     I2C_W_SCL(i2c, 0); // 拉低时钟，结束应答位
    
//     return 0; // 假装收到 ACK 了
// }


// /**
//  * @brief  I2C 总线死锁解除 (适配你的宏定义版)
//  * @note   发送最多9个脉冲，检测到 SDA 释放即停止
//  */
// void I2C_Fix_Deadlock(SoftI2C_Type* i2c)
// {
//     GPIO_InitTypeDef GPIO_InitStruct = {0}; // 结构体清零，防止垃圾值

//     // 1. 开启 GPIO 时钟
//     // 注意：如果你换了端口(如GPIOA)，这里记得要改时钟使能！
//     // 建议加个判断，或者简单粗暴地把你要用的都开了
//     if(i2c->port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
//     else if(i2c->port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
//     else if(i2c->port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
//     else if(i2c->port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
//     else if(i2c->port == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
//     else if(i2c->port == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
//     else if(i2c->port == GPIOG) __HAL_RCC_GPIOG_CLK_ENABLE();
//     // ...

//     // 2. 配置为开漏输出 (Open-Drain)
//     // 必须强制使用开漏，防止 OLED 拉低时我们也输出高导致短路
//     GPIO_InitStruct.Pin = i2c->scl_pin | i2c->sda_pin;
//     GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; 
//     GPIO_InitStruct.Pull = GPIO_PULLUP;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//     HAL_GPIO_Init(i2c->port, &GPIO_InitStruct);

//     // 3. 尝试先释放总线 (拉高)
//     I2C_W_SCL(i2c, 1);
//     I2C_W_SDA(i2c, 1);
//     HAL_Delay(1);

//     // 4. 发送脉冲 (最多9次，SDA 松手即停)
//     for(int i = 0; i < 9; i++)
//     {
//         // A. 检测 SDA 状态
//         // 如果读回来是 1 (SET)，说明 OLED 已经松手，死锁解除！
//         if(I2C_R_SDA(i2c) == GPIO_PIN_SET)
//         {
//             break; // 任务完成，跳出循环
//         }

//         // B. 如果还是低，给一个时钟脉冲推它一下
//         I2C_W_SCL(i2c, 0);
//         HAL_Delay(1); // 保持低电平 1ms
//         I2C_W_SCL(i2c, 1);
//         HAL_Delay(1); // 保持高电平 1ms
//     }

//     // 5. 发送标准 STOP 信号
//     // 确保 SCL 为低，准备 STOP 序列
//     I2C_W_SCL(i2c, 0);
//     I2C_W_SDA(i2c, 0); // SDA 拉低
//     HAL_Delay(1);
    
//     I2C_W_SCL(i2c, 1); // SCL 拉高 (建立 STOP 条件)
//     HAL_Delay(1);
    
//     I2C_W_SDA(i2c, 1); // SDA 拉高 (产生上升沿 -> STOP)
//     HAL_Delay(10); // 给一点恢复时间
// }

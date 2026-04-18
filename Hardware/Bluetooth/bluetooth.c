#include "bluetooth.h"
#include <string.h> // 用于 memcpy
#include <stdio.h>  // 用于 sprintf
#include <main.h> 
#include "usart.h" // 必须包含，以便获取 huart1 定义

// ================= 用户配置区 (User Configuration) =================

// 1. 硬件接口配置
// 如果以后改用串口2，只需要把这里改成 &huart2
#define BT_UART_HANDLE    &huart2

// 2. 接收数据配置 (提到前面了！)
// 对应你 App 设置的 "数据数量" (float 是 4 字节)
// 如果你以后想一次收 2 个 float，就把这里改成 8
#define RX_PAYLOAD_LEN    12       

uint32_t Last_Bluetooth_Time = 0; // 用于断连保护

// =================================================================

// --- 私有变量 ---
static uint8_t rx_buffer;                 // 1字节接收缓存
static uint8_t rx_state = 0;              // 状态机
uint8_t rx_data_cnt = 0;           // 计数器
static uint8_t rx_payload[RX_PAYLOAD_LEN];// 数据载荷数组 (长度由 .h 宏定义决定)

// 联合体：Float <-> Byte 互转
typedef union {
    float f_data;
    uint8_t b_data[4];
} FloatByteUnion;

/**
  * @brief  初始化蓝牙
  */
void Bluetooth_Init(void)
{
    // 开启中断接收 1 个字节
    HAL_UART_Receive_IT(BT_UART_HANDLE, &rx_buffer, 1);
}

/**
  * @brief  发送一个 float 数据到手机 App
  * @note   协议格式：0xA5(头) + data(4byte) + checksum + 0x5A(尾)
  */
void Bluetooth_SendFloat(float data)
{
    uint8_t tx_buf[7]; // 总长度 = 1(头) + 4(数据) + 1(校验) + 1(尾)
    uint8_t i;
    uint8_t check_sum = 0;
    FloatByteUnion send_union;

    send_union.f_data = data;

    // 1. 填充包头
    tx_buf[0] = 0xA5;

    // 2. 填充数据 & 计算校验和
    for(i = 0; i < 4; i++)
    {
        tx_buf[1 + i] = send_union.b_data[i];
        check_sum += send_union.b_data[i];
    }

    // 3. 填充校验位 (根据你的App协议，校验和是数据的低8位)
    tx_buf[5] = check_sum;

    // 4. 填充包尾
    tx_buf[6] = 0x5A;

    // 5. 发送
    HAL_UART_Transmit(BT_UART_HANDLE, tx_buf, 7, 10);
}

/**
  * @brief  发送字符串 (用于调试打印)
  */
void Bluetooth_SendString(char *str)
{
    HAL_UART_Transmit(BT_UART_HANDLE, (uint8_t *)str, strlen(str), 50);
}


/**
  * @brief  蓝牙断连保护 (建议放在 main.c 的 while(1) 中调用)
  */
void Check_Bluetooth_Alive(void)
{
    // 如果超过 500ms 没收到有效的蓝牙包，视为断连，强制停车
    if (HAL_GetTick() - Last_Bluetooth_Time > 500)
    {
        target_vx = 0.0f;
        target_vy = 0.0f;
        target_wz = 0.0f;
    }
}

/**
  * @brief  串口接收中断回调函数 (状态机解析)
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == BT_UART_HANDLE)
    {
        static uint8_t check_sum = 0;

        switch (rx_state)
        {
            case 0: // 等待包头 0xA5
                if (rx_buffer == 0xA5)
                {
                    rx_state = 1;
                    check_sum = 0;
                    rx_data_cnt = 0;
                }
                break;

            case 1: // 接收数据 (速度和转向)
                rx_payload[rx_data_cnt] = rx_buffer;
                check_sum += rx_buffer; // 累加校验和
                rx_data_cnt++;
                
                if (rx_data_cnt >= RX_PAYLOAD_LEN)
                {
                    rx_state = 2; // 数据收完，进入校验阶段
                }
                break;

            case 2: // 校验位比对 (对比低8位)
                if (rx_buffer == (check_sum & 0xFF))
                {
                    rx_state = 3;
                }
                else 
                {
                    rx_state = 0; // 校验失败，丢弃该包
                }
                break;

            case 3: // 等待包尾 0x5A
                if (rx_buffer == 0x5A)
                {
                    // === 成功接收完整数据包 ===
                    // 强转为有符号整数 (int8_t 范围是 -128 到 127)
                    FloatByteUnion vx_union, vy_union, wz_union;
                    memcpy(vx_union.b_data, &rx_payload[0], 4);
                    memcpy(vy_union.b_data, &rx_payload[4], 4);
                    memcpy(wz_union.b_data, &rx_payload[8], 4);
                    target_vx = vx_union.f_data;
                    target_vy = vy_union.f_data;
                    target_wz = wz_union.f_data;

                    // 摇杆死区消除：防止手机 App 摇杆回中时带有微小残余值 (-5 到 5 视为 0)
                    if(target_vx > -5 && target_vx < 5) target_vx = 0;
                    if(target_vy > -5 && target_vy < 5) target_vy = 0;
                    if(target_wz > -5 && target_wz < 5) target_wz = 0;

                    // 更新最后一次收到数据的时间
                    Last_Bluetooth_Time = HAL_GetTick(); 
                }
                rx_state = 0; // 回到初始状态，等待下一个包
                break;
                
            default:
                rx_state = 0;
                break;
        }

        // 重新开启中断接收下一个字节
        HAL_UART_Receive_IT(BT_UART_HANDLE, &rx_buffer, 1);
    }
}
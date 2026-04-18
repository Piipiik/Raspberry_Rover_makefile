#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include <stdint.h>

// 声明全局变量，供 PID 使用
extern float target_vx, target_vy, target_wz;

void Bluetooth_Init(void);
void Bluetooth_SendFloat(float data); 
void Bluetooth_SendString(char *str); 
void Check_Bluetooth_Alive();

#endif
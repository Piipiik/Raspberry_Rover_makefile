#ifndef __OLED_H
#define __OLED_H

#include "main.h"  // 包含HAL库或你的芯片头文件
#include "bsp_soft_i2c.h" // [移植说明] 这里换成你的I2C驱动头文件
#include <stdint.h>
#include <string.h> // 引入 memset, memcpy


/* ============================================================
   [移植修改区] 根据你的硬件连接和屏幕参数修改这里
   ============================================================ */

// 显存大小 (页数 * 列数)
extern uint8_t OLED_GRAM[8][128];

/* ============================================================
   [结构体定义]
   ============================================================ */

// 动画小球结构体 (供 main.c 使用)
typedef struct {
    float x;      // 当前X坐标 (float保证移动平滑)
    float y;      // 当前Y坐标
    float vx;     // X轴速度
    float vy;     // Y轴速度
    uint8_t r;    // 半径
} Ball;

// 滚动方向枚举
typedef enum {
    SCROLL_RIGHT = 0x26,
    SCROLL_LEFT  = 0x27
} OledScrollDir;

// 滚动速度枚举 (帧间隔)
typedef enum {
    SCROLL_SPEED_5_FRAMES   = 0x00, // 最快 (每5帧动一次)
    SCROLL_SPEED_64_FRAMES  = 0x01,
    SCROLL_SPEED_128_FRAMES = 0x02,
    SCROLL_SPEED_256_FRAMES = 0x03, // 慢
    SCROLL_SPEED_3_FRAMES   = 0x04, // 极快！(慎用，可能会花)
    SCROLL_SPEED_4_FRAMES   = 0x05,
    SCROLL_SPEED_25_FRAMES  = 0x06,
    SCROLL_SPEED_2_FRAMES   = 0x07  // 极极快
} OledScrollSpeed;

/* ============================================================
   [函数声明] 供外部调用的接口
   ============================================================ */

// [移植说明] I2C写命令接口
void OLED_Write_Cmd(uint8_t Cmd);

// [移植说明] I2C写数据接口
void OLED_Write_Data(uint8_t Data);

/**
 * @brief  初始化 OLED 屏幕
 * @note   包含软件配置和硬件复位(如有)
 */
void OLED_Init(SoftI2C_Type *i2c_bus);

/**
 * @brief  清空屏幕 (显存全黑)
 */
void OLED_Clear(uint8_t m);

/**
 * @brief 局部区域清空 (挖空/擦除)
 * @param x: 起始 x 坐标
 * @param y: 起始 y 坐标
 * @param w: 宽度
 * @param h: 高度
 * @param m: 1=立即刷新屏幕, 0=只改显存
 */
void OLED_Clear_Part(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t m);

/**
 * @brief  全屏刷新
 * @note   将显存数组的内容一次性写入 OLED
 */
void OLED_Refresh(void);

/**
 * @brief  局部刷新 (高级功能)
 * @param  x: 起始列 (0~127)
 * @param  w: 刷新宽度 (像素)
 * @param  y: 起始页 (0~7)
 * @param  h: 刷新高度 (页数! 1页=8像素)
 */
void OLED_Refresh_Part(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

/**
 * @brief  画点函数
 * @param  x, y: 像素坐标 (0~127, 0~63)
 * @param  t: 1=点亮, 0=熄灭
 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t);

/**
 * @brief  显示全屏位图
 * @param  image: 取模软件生成的数组指针 (必须是1024字节)
 */
void OLED_ShowImage(const uint8_t *image);

/* --- 字符与数字显示函数 --- */
// m 参数说明: 1=立即刷新到屏幕, 0=只写入显存不刷新

void OLED_ShowCharF8X16(uint8_t x, uint8_t y, uint8_t chr, uint8_t m);
void OLED_ShowCharF6X8(uint8_t x, uint8_t y, uint8_t chr, uint8_t m);

void OLED_ShowStringF8X16(uint8_t x, uint8_t y, char* String, uint8_t m);
void OLED_ShowStringF6X8(uint8_t x, uint8_t y, char* String, uint8_t m);

void OLED_ShowNumF6X16(uint8_t x, uint8_t y, uint32_t Num, uint8_t Length, uint8_t m);
void OLED_ShowNumF6X8(uint8_t x, uint8_t y, uint32_t Num, uint8_t Length, uint8_t m);

void OLED_ShowSignedNumF6X16(uint8_t x, uint8_t y, int32_t Num, uint8_t Length, uint8_t m);
void OLED_ShowSignedNumF6X8(uint8_t x, uint8_t y, int32_t Num, uint8_t Length, uint8_t m);

void OLED_ShowFloatF6X16(uint8_t x, uint8_t y, float Number, uint8_t Int_Len, uint8_t Dec_Len, uint8_t m);
void OLED_ShowFloatF6X8(uint8_t x, uint8_t y, float Number, uint8_t Int_Len, uint8_t Dec_Len, uint8_t m);

void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t no, uint8_t m);


void OLED_InvertArea(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t m);

void OLED_HardwareStartScroll(OledScrollDir dir, uint8_t start, uint8_t end, OledScrollSpeed speed);
void OLED_StopScroll(void);

void OLED_ShowPicture(int x, int y, uint8_t w, uint8_t h, const uint8_t* BMP);
void OLED_Animate_Move(int x_start, int y_start, int x_end, int y_end, const uint8_t* bmp, uint8_t w, uint8_t h, int speed_delay);

// 测试Demo
void Draw_Animation_Test(void);

#endif
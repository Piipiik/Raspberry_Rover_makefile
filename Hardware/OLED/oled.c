#include "oled.h"
#include "oled_font.h"    // 字库文件

// 核心：定义一个内部指针，用来记录当前 OLED 使用的是哪条 I2C 总线
static SoftI2C_Type *g_oled_i2c = NULL; 
static uint8_t g_oled_addr = 0x78; // 顺便把地址也存了

/* * 全局显存数组 (Buffer)
 * 格式：8页(行) x 128列
 * 作用：所有的画点、写字都先操作这个数组，最后通过 Refresh 发送给屏幕
 */
uint8_t OLED_GRAM[8][128];

/* ============================================================
   [内部工具函数] 不对外开放，仅本文件使用
   ============================================================ */

// 输入: m^n
uint32_t OLED_Pow(uint8_t m, uint8_t n) {
    uint32_t result = 1;
    while(n--) result *= m;
    return result;
}

// [移植说明] I2C写命令接口
void OLED_Write_Cmd(uint8_t Cmd) {
    MyI2C_Start(g_oled_i2c);
    MyI2C_SendByte(g_oled_i2c, g_oled_addr);
    MyI2C_WaitAck(g_oled_i2c);
    MyI2C_SendByte(g_oled_i2c, 0x00); // 0x00 表示后面是命令
    MyI2C_WaitAck(g_oled_i2c);
    MyI2C_SendByte(g_oled_i2c, Cmd);
    MyI2C_WaitAck(g_oled_i2c);
    MyI2C_Stop(g_oled_i2c);
}

// [移植说明] I2C写数据接口
void OLED_Write_Data(uint8_t Data) {
    MyI2C_Start(g_oled_i2c);
    MyI2C_SendByte(g_oled_i2c, g_oled_addr);
    MyI2C_WaitAck(g_oled_i2c);
    MyI2C_SendByte(g_oled_i2c, 0x40); // 0x40 表示后面是数据
    MyI2C_WaitAck(g_oled_i2c);
    MyI2C_SendByte(g_oled_i2c, Data);
    MyI2C_WaitAck(g_oled_i2c);
    MyI2C_Stop(g_oled_i2c);
}

/* ============================================================
   [核心驱动层] 初始化与刷新逻辑
   ============================================================ */

void OLED_Init(SoftI2C_Type *i2c_bus) {
    //HAL_Delay(800);

    g_oled_i2c = i2c_bus;

    MyI2C_Init(g_oled_i2c);

    const uint8_t Init_Cmds[] = {
        0xAE, 0xAE, 0xAE, 0xAE, 0xAE,
        
        0xAE,       // 关显示
        0x00, 0x10, // 设置列地址 (商家代码里的，虽然水平模式下会被覆盖，但加上无妨)
        0xB0,       // 页地址
        0x40,       // 起始行   0x40 到 0x7F (0~63) 放进for可以实现上下滚动特效
        0x81, 0xFF, // 【商家修改】对比度直接拉满到 0xFF (最亮)！
                    // 之前是 CF，现在改 FF，保证够亮
        0xA6,           // 正常显示 0xA6  // 反色显示 0xA7
        0xA8, 0x3F, // 多路复用    所用OLED宽高
        0xA1,          // 左右翻转 0xA1    0xA0
        0xC8,          // 上下翻转 0xC8    0xC0
        0xD3, 0x00, // 偏移    异性屏校准行
        // --- 商家特有配置区 ---
        0xD5, 0x80, // 时钟分频 (必须有！)  0x80 是默认值（约 100fps）
        0xD8, 0x05, // 很多国产廉价屏用的是兼容芯片（如 SH1106 或其他魔改版），如果不加这句，可能会出现屏幕右边有花点，或者休眠唤醒后死机
        0xD9, 0xF1, // 预充电      屏幕亮度不均调节 0xF1 或 0x22
        0xDA, 0x12, // COM引脚  硬核的物理配置，描述了玻璃基板上的引脚是“交错排列”还是“顺序排列”
        0xDB, 0x30, // VCOMH    调节 OLED 像素点在“熄灭”状态下的截止电压    0x20: ~0.77 x Vcc (低)  0x30: ~0.83 x Vcc (中，默认值)  0x40: ~1.00 x Vcc (高)
        0x8D, 0x14, // 电荷泵 (必须是 14)   开启升压电路
        // --- 适配你的 Refresh 函数 ---
        0x20, 0x00, // 商家代码没写这个，是因为他用页写入。
                    // 但你的代码是全屏刷新，必须加这个“水平寻址模式”，
                    // 否则你的 OLED_Refresh 会乱码！
        0xAF        // 开显示
    };

    uint8_t i;
    MyI2C_Start(g_oled_i2c);
    MyI2C_SendByte(g_oled_i2c, g_oled_addr);
    MyI2C_WaitAck(g_oled_i2c);
    MyI2C_SendByte(g_oled_i2c, 0x00); // 开启连续命令模式
    MyI2C_WaitAck(g_oled_i2c);
    for(i = 0; i < sizeof(Init_Cmds); i++) {
        MyI2C_SendByte(g_oled_i2c, Init_Cmds[i]);
        MyI2C_WaitAck(g_oled_i2c);
    }
    MyI2C_Stop(g_oled_i2c);
    
    OLED_Clear(1); // 上电清屏
}

// 全屏刷新
void OLED_Refresh(void) {
    uint8_t *pRAM = (uint8_t *)OLED_GRAM; 
    uint16_t i;

    // 1. 设置写入窗口为全屏
    OLED_Write_Cmd(0x21); OLED_Write_Cmd(0x00); OLED_Write_Cmd(0x7F);
    OLED_Write_Cmd(0x22); OLED_Write_Cmd(0x00); OLED_Write_Cmd(0x07);

    // 2. 连续写入1024个字节的数据
    MyI2C_Start(g_oled_i2c);
    MyI2C_SendByte(g_oled_i2c, g_oled_addr); MyI2C_WaitAck(g_oled_i2c);
    MyI2C_SendByte(g_oled_i2c, 0x40);      MyI2C_WaitAck(g_oled_i2c); // 数据模式

    for (i = 0; i < 1024; i++) {
        MyI2C_SendByte(g_oled_i2c, pRAM[i]);
        MyI2C_WaitAck(g_oled_i2c);
    }
    MyI2C_Stop(g_oled_i2c);
}

// 局部刷新 (提高帧率的关键)
void OLED_Refresh_Part(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    uint8_t i, n;

    // 1. 设置限制窗口 (利用硬件特性自动换行)
    OLED_Write_Cmd(0x21); OLED_Write_Cmd(x); OLED_Write_Cmd(x + w - 1);
    OLED_Write_Cmd(0x22); OLED_Write_Cmd(y); OLED_Write_Cmd(y + h - 1);

    // 2. 写入窗口内的数据
    MyI2C_Start(g_oled_i2c);
    MyI2C_SendByte(g_oled_i2c, g_oled_addr); MyI2C_WaitAck(g_oled_i2c);
    MyI2C_SendByte(g_oled_i2c, 0x40);      MyI2C_WaitAck(g_oled_i2c);

    for(i = 0; i < h; i++) {       // 遍历页
        for(n = 0; n < w; n++) {   // 遍历列
            MyI2C_SendByte(g_oled_i2c, OLED_GRAM[y + i][x + n]);
            MyI2C_WaitAck(g_oled_i2c);
        }
    }
    MyI2C_Stop(g_oled_i2c);
}

void OLED_Clear(uint8_t m) {
    // 使用标准库 memset 极速清零，比 for 循环快
    if (m) {
            memset(OLED_GRAM, 0, sizeof(OLED_GRAM)); 
            OLED_Refresh();        
    } else {
            memset(OLED_GRAM, 0, sizeof(OLED_GRAM)); 
    }
}

/**
 * @brief 局部区域清空 (挖空/擦除)
 * @param x: 起始 x 坐标
 * @param y: 起始 y 坐标
 * @param w: 宽度
 * @param h: 高度
 * @param m: 1=立即刷新屏幕, 0=只改显存
 */
void OLED_Clear_Part(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t m)  {
    uint8_t i, j;
    uint8_t page, bit_mask;

    // --- 1. 操作显存 (GRAM) ---
    // 外层循环：遍历 Y 轴 (行)
    for (j = y; j < (y + h); j++) 
    {
        // 边界保护
        if (j >= 64) break;

        // 【优化】循环不变式外提
        page = j / 8;
        
        // 【关键点】制作清零掩码
        // 1 << (j%8) 生成的是 00001000 (要把这一位清零)
        // 取反 (~) 后变成     11110111 (除了这一位是0，其他都是1)
        // 这样用 & (与) 运算后，只有这一位会被强制拉低为0，其他位保持不变
        bit_mask = ~(1 << (j % 8)); 

        // 内层循环：遍历 X 轴 (列)
        for (i = x; i < (x + w); i++) 
        {
            // 边界保护
            if (i >= 128) break;

            // 使用“按位与” + “掩码”进行清零
            OLED_GRAM[page][i] &= bit_mask;
        }
    }

    // --- 2. 局部刷新 (如果 m=1) ---
    if (m) 
    {
        uint8_t pg_start = y / 8;
        uint8_t pg_end   = (y + h - 1) / 8;
        
        if (pg_end > 7) pg_end = 7;

        // 调用你的局部刷新函数
        OLED_Refresh_Part(x, pg_start, w, pg_end - pg_start + 1);
    }
}

/* ============================================================
   [绘图层] 点、图
   ============================================================ */

void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t) {
    if (x > 127 || y > 63) return; // 边界保护
    
    // 显存结构：1个字节管8个竖着的像素
    uint8_t page = y / 8;       // 算页
    uint8_t bit_offset = y % 8; // 算位

    if (t) OLED_GRAM[page][x] |= (1 << bit_offset);     // 置1 (亮)
    else   OLED_GRAM[page][x] &= ~(1 << bit_offset);    // 置0 (灭)
}

void OLED_ShowImage(const uint8_t *image) {
    // 同样使用标准库 memcpy 进行内存拷贝
    memcpy(OLED_GRAM, image, sizeof(OLED_GRAM));
    OLED_Refresh();
}

/* ============================================================
   [应用层] 字符与数字显示 (核心算法)
   ============================================================ */

// 显示单个字符 F8X16
void OLED_ShowCharF8X16(uint8_t x, uint8_t y, uint8_t chr, uint8_t m) {
    uint8_t i, c = chr - ' '; // 得到偏移值
    if (x > 120 || y > 6 || c > 94) return; // 边界保护

    for (i = 0; i < 8; i++) {
        OLED_GRAM[y][x + i] = F8X16[c][i];
        OLED_GRAM[y + 1][x + i] = F8X16[c][i + 8];
    }
    if (m) OLED_Refresh_Part(x, y, 8, 2); // 高度2页
}

// 显示单个字符 F6X8
void OLED_ShowCharF6X8(uint8_t x, uint8_t y, uint8_t chr, uint8_t m) {
    uint8_t i, c = chr - ' ';
    if (x > 122 || y > 7 || c > 94) return;

    for (i = 0; i < 6; i++) {
        OLED_GRAM[y][x + i] = F6X8[c][i];
    }
    if (m) OLED_Refresh_Part(x, y, 6, 1); // 高度1页
}

// 字符串显示 F8X16
void OLED_ShowStringF8X16(uint8_t x, uint8_t y, char* String, uint8_t m) {
    uint8_t i, j, chr;
    uint8_t len = strlen(String);

    for (i = 0; i < len; i++) {
        chr = String[i] - ' ';
        if ((x + i * 8) > 120 || y > 6) return;

        for (j = 0; j < 8; j++) {
            OLED_GRAM[y][x + i * 8 + j] = F8X16[chr][j];
            OLED_GRAM[y + 1][x + i * 8 + j] = F8X16[chr][j + 8];
        }
    }
    if (m) OLED_Refresh_Part(x, y, 8 * len, 2);
}

// 显示字符串 F6X8
void OLED_ShowStringF6X8(uint8_t x, uint8_t y, char* String, uint8_t m) {
    uint8_t i, j, chr;
    uint8_t len = strlen(String);

    for (i = 0; i < len; i++) {
        chr = String[i] - ' ';
        // 边界保护：x + 当前字符偏移 > 127
        if ((x + i * 6) > 122 || y > 7) return;

        for (j = 0; j < 6; j++) {
            OLED_GRAM[y][x + i * 6 + j] = F6X8[chr][j];
        }
    }
    if (m) OLED_Refresh_Part(x, y, 6 * len, 1);
}

/**
 * @brief  内部函数：显示无符号数字的核心逻辑
 * @param  UnsignedNum: 正整数
 */
static void Show_Num_Core(uint8_t x, uint8_t y, uint32_t UnsignedNum, uint8_t Len, uint8_t IsF6X16) {
    uint8_t i, j, num;
    for (i = 0; i < Len; i++) {
        // 核心算法：提取第 i 位数字 (高位在前)
        num = (UnsignedNum / OLED_Pow(10, Len - i - 1)) % 10;
        
        if (IsF6X16) {
            // 6x16 字体处理 (占2页)
            for (j = 0; j < 6; j++) {
                OLED_GRAM[y][x + i * 6 + j] = Font_Num_6X16[num][j];
                OLED_GRAM[y + 1][x + i * 6 + j] = Font_Num_6X16[num][j + 6];
            }
        } else {
            // 5x8 字体处理 (占1页, 需补间隔)
            uint8_t pos_x = x + i * 6;
            for (j = 0; j < 5; j++) OLED_GRAM[y][pos_x + j] = Font_Num_5X8[num][j];
            OLED_GRAM[y][pos_x + 5] = 0; // 补第6列空白
        }
    }
}

// 显示F6x16 无符号数字
void OLED_ShowNumF6X16(uint8_t x, uint8_t y, uint32_t Num, uint8_t Length, uint8_t m) {
    Show_Num_Core(x, y, Num, Length, 1);
    if (m) OLED_Refresh_Part(x, y, 6 * Length, 2); // 16像素高 = 2页
}

// 显示F5x8 无符号数字
void OLED_ShowNumF6X8(uint8_t x, uint8_t y, uint32_t Num, uint8_t Length, uint8_t m) {
    Show_Num_Core(x, y, Num, Length, 0);
    if (m) OLED_Refresh_Part(x, y, 6 * Length, 1); // 8像素高 = 1页
}

// 显示 F6x16 有符号数字
void OLED_ShowSignedNumF6X16(uint8_t x, uint8_t y, int32_t Num, uint8_t Length, uint8_t m) {
    uint8_t sym, j;
    uint32_t UnsignedNum;

    // 符号处理
    if (Num >= 0) { sym = 11; UnsignedNum = Num; }
    else          { sym = 12; UnsignedNum = -Num; } // 负数转正

    // 画符号
    for (j = 0; j < 6; j++) {
        OLED_GRAM[y][x + j] = Font_Num_6X16[sym][j];
        OLED_GRAM[y + 1][x + j] = Font_Num_6X16[sym][j + 6];
    }
    // 画数字 (偏移6个像素)
    Show_Num_Core(x + 6, y, UnsignedNum, Length, 1);
    
    if (m) OLED_Refresh_Part(x, y, 6 * (Length + 1), 2);
}

// 显示 F5x8 有符号数字
void OLED_ShowSignedNumF6X8(uint8_t x, uint8_t y, int32_t Num, uint8_t Length, uint8_t m) {
    uint8_t sym, j;
    uint32_t UnsignedNum;

    // 1. 符号处理
    if (Num >= 0) { sym = 11; UnsignedNum = Num; }
    else          { sym = 12; UnsignedNum = -Num; }

    // 2. 画符号 (F5x8 字宽5 + 间隔1)
    for (j = 0; j < 5; j++) OLED_GRAM[y][x + j] = Font_Num_5X8[sym][j];
    OLED_GRAM[y][x + 5] = 0; // 补间隔

    // 3. 画数字 (利用核心函数，偏移6个像素)
    // Show_Num_Core(x坐标, y坐标, 数字, 长度, 是否F6X16模式)
    Show_Num_Core(x + 6, y, UnsignedNum, Length, 0); // 0 代表 F5X8 模式

    if (m) OLED_Refresh_Part(x, y, 6 * (Length + 1), 1); // 高度1页
}

// 显示 F6x16 浮点数
void OLED_ShowFloatF6X16(uint8_t x, uint8_t y, float Number, uint8_t Int_Len, uint8_t Dec_Len, uint8_t m) {
    uint8_t sym, j;
    
    // 1. 符号处理
    if (Number < 0) { sym = 12; Number = -Number; }
    else            { sym = 11; }

    // 画符号
    for(j=0; j<6; j++) {
         OLED_GRAM[y][x+j] = Font_Num_6X16[sym][j];
         OLED_GRAM[y+1][x+j] = Font_Num_6X16[sym][j+6];
    }

    // 2. 整数部分
    uint32_t Int_Part = (uint32_t)Number;
    Show_Num_Core(x + 6, y, Int_Part, Int_Len, 1);

    // 3. 小数部分
    if(Dec_Len > 0) {
        // 四舍五入提取小数
        uint32_t Dec_Part = (uint32_t)((Number - Int_Part) * OLED_Pow(10, Dec_Len) + 0.5);
        
        // 画小数点 (在整数后面)
        uint8_t dot_x = x + 6 + Int_Len * 6;
        for(j=0; j<6; j++) {
            OLED_GRAM[y][dot_x+j] = Font_Num_6X16[10][j]; // 假设索引10是点
            OLED_GRAM[y+1][dot_x+j] = Font_Num_6X16[10][j+6];
        }

        // 画小数数字 (在点后面)
        Show_Num_Core(dot_x + 6, y, Dec_Part, Dec_Len, 1);
    }
    
    if (m) {
        uint8_t total_w = 6 + Int_Len*6 + (Dec_Len > 0 ? (6 + Dec_Len*6) : 0);
        OLED_Refresh_Part(x, y, total_w, 2);
    }
}

// 显示浮点数 F5X8
void OLED_ShowFloatF6X8(uint8_t x, uint8_t y, float Number, uint8_t Int_Len, uint8_t Dec_Len, uint8_t m) {
    uint8_t sym, j;
    
    // 1. 符号处理
    if (Number < 0) { sym = 12; Number = -Number; }
    else            { sym = 11; } // '+' 或 ' '

    // 画符号
    for (j = 0; j < 5; j++) OLED_GRAM[y][x + j] = Font_Num_5X8[sym][j];
    OLED_GRAM[y][x + 5] = 0;

    // 2. 整数部分
    uint32_t Int_Part = (uint32_t)Number;
    // 调用核心函数显示整数 (偏移6像素)
    Show_Num_Core(x + 6, y, Int_Part, Int_Len, 0);

    // 3. 小数部分
    if(Dec_Len > 0) {
        // 计算小数数值
        uint32_t Dec_Part = (uint32_t)((Number - Int_Part) * OLED_Pow(10, Dec_Len) + 0.5);
        
        // 画小数点 (假设索引10是点)
        uint8_t dot_x = x + 6 + Int_Len * 6;
        for (j = 0; j < 5; j++) OLED_GRAM[y][dot_x + j] = Font_Num_5X8[10][j];
        OLED_GRAM[y][dot_x + 5] = 0;

        // 画小数数字 (偏移: 符号6 + 整数长*6 + 小数点6)
        Show_Num_Core(dot_x + 6, y, Dec_Part, Dec_Len, 0);
    }
    
    // 4. 刷新
    if (m) {
        uint8_t total_w = 6 + Int_Len * 6 + (Dec_Len > 0 ? (6 + Dec_Len * 6) : 0);
        OLED_Refresh_Part(x, y, total_w, 1); // 高度1页
    }
}

// 汉字显示 (16x16)
void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t no, uint8_t m) {
    uint8_t i;
    if (x > 112 || y > 6) return;

    for (i = 0; i < 16; i++) {
        OLED_GRAM[y][x + i] = Hz_16X16[no][i];     // 上半字
        OLED_GRAM[y + 1][x + i] = Hz_16X16[no][i + 16]; // 下半字
    }

    if (m) OLED_Refresh_Part(x, y, 16, 2);
}

/**
 * @brief 局部反色函数 (高性能优化版)
 * @param x: 起始 x 坐标 (0~127)
 * @param y: 起始 y 坐标 (0~63)
 * @param w: 区域宽度
 * @param h: 区域高度
 * @param m: 刷新模式 (1=立即刷新屏幕, 0=只改显存不刷新)
 */
void OLED_InvertArea(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t m)  {
    uint8_t i, j;
    uint8_t page, bit_mask;
    
    // --- 1. 修改显存 (GRAM) ---
    // 外层循环控制“行”(y坐标)
    for (j = y; j < (y + h); j++) 
    {
        // 边界安全检查：如果 Y 坐标超出屏幕高度(64)，直接停止，防止内存溢出
        if (j >= 64) break;

        page = j / 8;          
        bit_mask = 1 << (j % 8); 

        // 内层循环控制“列”(x坐标)
        for (i = x; i < (x + w); i++) 
        {
            // 边界安全检查：如果 X 坐标超出屏幕宽度(128)，停止画这一行
            if (i >= 128) break;

            // 直接异或，极速操作
            OLED_GRAM[page][i] ^= bit_mask;
        }
    }
    // --- 2. 局部刷新 (如需要) ---
    if (m) 
    {
        // 计算涉及到的起始页和结束页
        // 公式原理：(结束Y坐标) / 8 - (起始Y坐标) / 8 + 1
        uint8_t pg_start = y / 8;
        uint8_t pg_end   = (y + h - 1) / 8;
        
        // 防止页码计算溢出 (虽然前面的 break 已经保护了，这里为了逻辑严谨再加一道)
        if (pg_end > 7) pg_end = 7;

        // 调用你的局部刷新函数
        // 参数：X起始, 页起始, 宽度, 刷新的页数
        OLED_Refresh_Part(x, pg_start, w, pg_end - pg_start + 1);
    }
}

/**
 * @brief 开启硬件水平滚动
 * @param dir:   滚动方向 (SCROLL_RIGHT 或 SCROLL_LEFT)
 * @param start: 起始页 (0~7) -> 对应屏幕的第0行到第63行
 * @param end:   结束页 (0~7) -> 必须 >= start
 * @param speed: 滚动速度 (使用枚举值)
 */
void OLED_HardwareStartScroll(OledScrollDir dir, uint8_t start, uint8_t end, OledScrollSpeed speed) {
    // 1. 先停止当前的滚动，防止冲突
    OLED_Write_Cmd(0x2E); 

    // 2. 发送滚动指令包
    OLED_Write_Cmd(dir);     // 发送方向 (0x26 或 0x27)
    
    OLED_Write_Cmd(0x00);    // 【虚拟字节 A】(Dummy Byte)
    
    OLED_Write_Cmd(start);   // 起始页地址 (0-7)
    
    OLED_Write_Cmd(speed);   // 速度 (帧间隔)
    
    OLED_Write_Cmd(end);     // 结束页地址 (0-7)
    
    OLED_Write_Cmd(0x00);    // 【虚拟字节 B】(Dummy Byte)
    OLED_Write_Cmd(0xFF);    // 【虚拟字节 C】(Dummy Byte)
    
    // 3. 激活滚动
    OLED_Write_Cmd(0x2F); 
}

// 停止滚动 (恢复正常显示必须调用这个)
void OLED_StopScroll(void) {
    OLED_Write_Cmd(0x2E); 
}

/**
 * @brief 任意坐标显示图片 (高性能优化版)
 * @note 优化了乘法计算次数，增加了 X 轴快速跳过机制
 */
void OLED_ShowPicture(int x, int y, uint8_t w, uint8_t h, const uint8_t* BMP) {
    uint8_t page, col, bit;
    uint8_t temp_byte;
    
    // 这里的变量用于缓存计算结果，避免重复计算
    int page_base_y;  // 这一页 BMP 数据对应的屏幕起始 Y 坐标
    int bmp_offset;   // 这一页 BMP 数据在数组中的起始索引
    int draw_x, draw_y;

    // 1. 遍历图片的每一页
    for (page = 0; page < (h / 8); page++)
    {
        // 【优化1】循环不变式外提
        // 这些计算跟 col 和 bit 无关，提出来只算一次！
        page_base_y = y + (page * 8); 
        bmp_offset  = page * w;

        // 2. 遍历图片的每一列
        for (col = 0; col < w; col++)
        {
            draw_x = x + col;

            // 【优化2】X轴快速剪裁 (Guard Gate)
            // 如果这一列的 X 坐标已经在屏幕外了，
            // 根本没必要去取数据、没必要进 bit 循环、没必要算 draw_y
            // 直接跳过，省下 8 次内层循环的开销！
            if (draw_x < 0 || draw_x >= 128) continue;

            // 取出这 1 列的 8 个像素
            temp_byte = BMP[bmp_offset + col];

            // 3. 遍历 8 个像素点
            for (bit = 0; bit < 8; bit++)
            {
                // 使用外层算好的基准值，只做简单的加法
                draw_y = page_base_y + bit;

                // Y轴剪裁
                if (draw_y < 0 || draw_y >= 64) continue;

                // 核心画点逻辑 (这里没法再省了，必须逐点判断)
                // 编译器会自动把 /8 优化成 >>3，把 %8 优化成 &7，不用担心
                if (temp_byte & (0x01 << bit))
                {
                    OLED_GRAM[draw_y / 8][draw_x] |= (1 << (draw_y % 8));
                }
                else
                {
                    OLED_GRAM[draw_y / 8][draw_x] &= ~(1 << (draw_y % 8));
                }
            }
        }
    }
}

/**
 * @brief 简单的位图移动动画
 * @param x_start, y_start: 起始坐标
 * @param x_end, y_end:     终点坐标
 * @param bmp:              图片数组
 * @param w, h:             图片宽高
 * @param speed_delay:      移动每一步的延时(ms)，越小越快
 */
void OLED_Animate_Move(int x_start, int y_start, int x_end, int y_end, 
                       const uint8_t* bmp, uint8_t w, uint8_t h, int speed_delay) {
    int current_x = x_start;
    int current_y = y_start;
    
    // 简单的线性插值移动 (这里简化为每次移动 1-2 像素，直到到达终点)
    while (current_x != x_end || current_y != y_end)
    {
        // 1. 每次循环前，必须清除显存 (或者只清除上一次画图的区域)
        //    如果只清除局部，效率更高，但逻辑复杂。这里用全屏清演示。
        OLED_Clear_Part(current_x,  current_y,  w,  h,  0);
        
        // 2. 计算下一步坐标
        if (current_x < x_end) current_x++;
        if (current_x > x_end) current_x--;
        
        if (current_y < y_end) current_y++;
        if (current_y > y_end) current_y--;
        
        // 3. 在新坐标画图
        // 注意：你的 ShowPicture 必须支持坐标裁剪(越界不画)，否则这里移出屏幕会死机
        OLED_ShowPicture(current_x, current_y, w, h, bmp);

        // 5. 刷新到屏幕
        OLED_Refresh();
        
        // 6. 控制速度
        HAL_Delay(speed_delay);
    }
}

// 测试 Demo
void Draw_Animation_Test(void) {
    Ball b = {64, 32, 2.5, 1.5, 3}; 

    while(1) {
        OLED_Clear(1); // 每一帧先清空
        
        b.x += b.vx; b.y += b.vy;
        
        // 边界反弹
        if (b.x <= b.r || b.x >= 127 - b.r) b.vx = -b.vx;
        if (b.y <= b.r || b.y >= 63 - b.r)  b.vy = -b.vy;

        // 简易画球 (正方形)
        for(uint8_t i = (uint8_t)(b.x - b.r); i <= (uint8_t)(b.x + b.r); i++) {
            for(uint8_t j = (uint8_t)(b.y - b.r); j <= (uint8_t)(b.y + b.r); j++) {
                OLED_DrawPoint(i, j, 1);
            }
        }
        OLED_Refresh();
        HAL_Delay(20); // 移植时解开这里的注释
    }
}
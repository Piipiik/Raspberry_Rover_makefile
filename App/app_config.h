/* App/app_config.h */
#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// // ================= 功能使能开关 (1=开启, 0=关闭) =================
// #define USE_OLED_DISPLAY        1   // 是否启用OLED屏幕
// #define USE_BLUETOOTH_COMM      1   // 是否启用蓝牙通信
// #define USE_PID_CONTROL          1   // 是否启用PID闭环控制
// #define USE_MONITOR_TASK         1   // 是否启用系统监控任务

// // ================= 调试配置 =================
// #define DEBUG_ENABLE             1   // 总调试开关

// // ================= 任务周期配置 (单位: 毫秒 ms) =================
// #define PERIOD_SENSOR_MS         5    // 传感器读取周期 (5ms, 200Hz)
// #define PERIOD_MOTION_MS         5    // 运动控制周期 (5ms, 200Hz)
// #define PERIOD_DISPLAY_MS        100  // 屏幕刷新周期 (100ms, 10Hz)
// #define PERIOD_INTERACT_MS       10   // 通信处理周期 (10ms)
// #define PERIOD_MONITOR_MS        500  // 系统监控周期 (500ms)

// // ================= 机器人物理参数 (示例) =================
// #define WHEEL_RADIUS             0.03f   // 轮子半径 (米)
// #define WHEEL_BASE               0.20f   // 轮距 (米)
// #define WHEEL_TRACK              0.18f   // 轴距 (米)
// #define ENCODER_RESOLUTION       1320.0f // 编码器分辨率 (CPR)
// #define REDUCTION_RATIO          30.0f   // 减速比

#ifdef __cplusplus
}
#endif

#endif /* __APP_CONFIG_H */
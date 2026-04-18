/* App/Monitor/app_monitor.h */
#ifndef __APP_MONITOR_H
#define __APP_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis_os.h"

/* 任务函数声明 */
void SystemGuardianTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __APP_MONITOR_H */
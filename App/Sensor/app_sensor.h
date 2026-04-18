/* App/Motion/app_motion.h */
#ifndef __APP_MOTION_H
#define __APP_MOTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis_os.h"

/* 任务函数声明 */
void SensorAcquisitionTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __APP_SENSOR_H */

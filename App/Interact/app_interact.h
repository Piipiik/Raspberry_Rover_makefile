/* App/Interact/app_interact.h */
#ifndef __APP_INTERACT_H
#define __APP_INTERACT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis_os.h"

/* 任务函数声明 */
void ProtocolProcessTask(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __APP_INTERACT_H */
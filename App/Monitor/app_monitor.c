/* App/Monitor/app_monitor.c */
#include "app_monitor.h"
#include "app.h"


void SystemGuardianTask(void *argument)
{
    for(;;) {
        osDelay(10);
    }
}
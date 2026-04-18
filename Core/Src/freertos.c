/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for Task_Initialize */
osThreadId_t Task_InitializeHandle;
uint32_t Task_InitializeBuffer[ 128 ];
osStaticThreadDef_t Task_InitializeControlBlock;
const osThreadAttr_t Task_Initialize_attributes = {
  .name = "Task_Initialize",
  .cb_mem = &Task_InitializeControlBlock,
  .cb_size = sizeof(Task_InitializeControlBlock),
  .stack_mem = &Task_InitializeBuffer[0],
  .stack_size = sizeof(Task_InitializeBuffer),
  .priority = (osPriority_t) osPriorityRealtime7,
};
/* Definitions for Task_Sensor */
osThreadId_t Task_SensorHandle;
uint32_t Task_SensorBuffer[ 128 ];
osStaticThreadDef_t Task_SensorControlBlock;
const osThreadAttr_t Task_Sensor_attributes = {
  .name = "Task_Sensor",
  .cb_mem = &Task_SensorControlBlock,
  .cb_size = sizeof(Task_SensorControlBlock),
  .stack_mem = &Task_SensorBuffer[0],
  .stack_size = sizeof(Task_SensorBuffer),
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for Task_Interact */
osThreadId_t Task_InteractHandle;
uint32_t Task_InteractBuffer[ 3000 ];
osStaticThreadDef_t Task_InteractControlBlock;
const osThreadAttr_t Task_Interact_attributes = {
  .name = "Task_Interact",
  .cb_mem = &Task_InteractControlBlock,
  .cb_size = sizeof(Task_InteractControlBlock),
  .stack_mem = &Task_InteractBuffer[0],
  .stack_size = sizeof(Task_InteractBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_Monitor */
osThreadId_t Task_MonitorHandle;
uint32_t Task_MonitorBuffer[ 128 ];
osStaticThreadDef_t Task_MonitorControlBlock;
const osThreadAttr_t Task_Monitor_attributes = {
  .name = "Task_Monitor",
  .cb_mem = &Task_MonitorControlBlock,
  .cb_size = sizeof(Task_MonitorControlBlock),
  .stack_mem = &Task_MonitorBuffer[0],
  .stack_size = sizeof(Task_MonitorBuffer),
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for Task_Display */
osThreadId_t Task_DisplayHandle;
uint32_t Task_DisplayBuffer[ 128 ];
osStaticThreadDef_t Task_DisplayControlBlock;
const osThreadAttr_t Task_Display_attributes = {
  .name = "Task_Display",
  .cb_mem = &Task_DisplayControlBlock,
  .cb_size = sizeof(Task_DisplayControlBlock),
  .stack_mem = &Task_DisplayBuffer[0],
  .stack_size = sizeof(Task_DisplayBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Task_Motion */
osThreadId_t Task_MotionHandle;
uint32_t Task_MotionBuffer[ 512 ];
osStaticThreadDef_t Task_MotionControlBlock;
const osThreadAttr_t Task_Motion_attributes = {
  .name = "Task_Motion",
  .cb_mem = &Task_MotionControlBlock,
  .cb_size = sizeof(Task_MotionControlBlock),
  .stack_mem = &Task_MotionBuffer[0],
  .stack_size = sizeof(Task_MotionBuffer),
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void InitializeTask(void *argument);
extern void SensorAcquisitionTask(void *argument);
extern void ProtocolProcessTask(void *argument);
extern void SystemGuardianTask(void *argument);
extern void UserInterfaceTask(void *argument);
extern void MotorControlTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);
void vApplicationDaemonTaskStartupHook(void);

/* USER CODE BEGIN 2 */
void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
void vApplicationTickHook( void )
{
   /* This function will be called by each tick interrupt if
   configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. User code can be
   added here, but the tick hook is called from an interrupt context, so
   code must not attempt to block, and only the interrupt safe FreeRTOS API
   functions can be used (those that end in FromISR()). */
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
   for(;;){
      HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_5); // 发生栈溢出时，PC13 翻转，方便调试
   }
   
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
}
/* USER CODE END 5 */

/* USER CODE BEGIN DAEMON_TASK_STARTUP_HOOK */
void vApplicationDaemonTaskStartupHook(void)
{
}
/* USER CODE END DAEMON_TASK_STARTUP_HOOK */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Task_Initialize */
  Task_InitializeHandle = osThreadNew(InitializeTask, NULL, &Task_Initialize_attributes);

  /* creation of Task_Sensor */
  Task_SensorHandle = osThreadNew(SensorAcquisitionTask, NULL, &Task_Sensor_attributes);

  /* creation of Task_Interact */
  Task_InteractHandle = osThreadNew(ProtocolProcessTask, NULL, &Task_Interact_attributes);

  /* creation of Task_Monitor */
  Task_MonitorHandle = osThreadNew(SystemGuardianTask, NULL, &Task_Monitor_attributes);

  /* creation of Task_Display */
  Task_DisplayHandle = osThreadNew(UserInterfaceTask, NULL, &Task_Display_attributes);

  /* creation of Task_Motion */
  Task_MotionHandle = osThreadNew(MotorControlTask, NULL, &Task_Motion_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_InitializeTask */
/**
  * @brief  Function implementing the Task_Initialize thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_InitializeTask */
void InitializeTask(void *argument)
{
  /* USER CODE BEGIN InitializeTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END InitializeTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */


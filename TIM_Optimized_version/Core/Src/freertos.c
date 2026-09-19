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
#include "bsp_led.h"
#include "bsp_key.h"
#include "bsp_log.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
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
/* 任务内存（静态分配）：栈数组 + 控制块 TCB
   ⚠️ 栈数组必须 8 字节对齐！本工程 FreeRTOS 的 portBYTE_ALIGNMENT = 8，
      prvInitialiseNewTask() 里有 configASSERT((pxStack & 7) == 0) 检查；
      动态分配时 pvPortMalloc 会自动对齐，静态分配就得自己声明 */
__align(8) static StackType_t ledTaskStack[256];      /* 256 word = 1KB */
static StaticTask_t ledTaskTCB;
TaskHandle_t ledTaskHandle = NULL;

__align(8) static StackType_t keyTaskStack[128];      /* 128 word = 512B */
static StaticTask_t keyTaskTCB;
TaskHandle_t keyTaskHandle = NULL;

__align(8) static StackType_t logTaskStack[256];
static StaticTask_t logTaskTCB;
TaskHandle_t logTaskHandle = NULL;
/* USER CODE END Variables */
/* Definitions for defaultTask */


/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
extern void xPortSysTickHandler(void);   /* FreeRTOS 的 tick 处理，定义在 port.c；头文件未暴露原型，这里自己声明 */
/* USER CODE END FunctionPrototypes */



void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

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
  (void)led_init();
  (void)key_init();
  (void)log_init();
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */


  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  ledTaskHandle = xTaskCreateStatic(led_task_func, "LEDTask", 256, NULL,
                                  tskIDLE_PRIORITY + 3, ledTaskStack, &ledTaskTCB);
  if (NULL == ledTaskHandle) { Error_Handler(); }

  keyTaskHandle = xTaskCreateStatic(key_task_func,"KEYTask",128,NULL,
                                    tskIDLE_PRIORITY + 5,keyTaskStack,&keyTaskTCB);
  if(NULL == keyTaskHandle){Error_Handler();}

  logTaskHandle = xTaskCreateStatic(log_task_func, "LogTask", 256, NULL,
                                  tskIDLE_PRIORITY + 7, logTaskStack, &logTaskTCB);
  if (NULL == logTaskHandle) { Error_Handler(); }
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */

/* USER CODE END Header_StartDefaultTask */

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */

  /* USER CODE END StartDefaultTask */


/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t  **ppxIdleTaskStackBuffer,
                                   uint32_t      *pulIdleTaskStackSize)
{
    /* ⚠️ 栈数组同样必须 8 字节对齐（理由见 USER CODE BEGIN Variables 处的注释） */
    __align(8) static StaticTask_t xIdleTaskTCB;
    __align(8) static StackType_t  uxIdleTaskStack[configMINIMAL_STACK_SIZE];   /* = 128 word */

    *ppxIdleTaskTCBBuffer   = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize   = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t  **ppxTimerTaskStackBuffer,
                                    uint32_t      *pulTimerTaskStackSize)
{
    __align(8) static StaticTask_t xTimerTaskTCB;
    __align(8) static StackType_t  uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];  /* = 256 word */

    *ppxTimerTaskTCBBuffer   = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize   = configTIMER_TASK_STACK_DEPTH;
}

/* FreeRTOS 的 tick 中断处理。
   ⚠️ 原来定义在 cmsis_os2.c 里，移除那个文件后必须自己提供；
      否则 SysTick 中断会回落到启动文件的 Default_Handler（死循环）→ 一进中断整机卡死。
   注：本工程 HAL 时基是 TIM1（main.c 的 HAL_TIM_PeriodElapsedCallback 里调 HAL_IncTick），
       所以这里不需要调 HAL_IncTick()。 */
void SysTick_Handler(void)
{
    SysTick->CTRL;      /* 读一下 CTRL 即可清除 SysTick 的溢出标志 */

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
}
/* USER CODE END Application */


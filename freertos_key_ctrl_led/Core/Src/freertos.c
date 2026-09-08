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
#include "bsp_key.h"
#include "queue.h"  //freertos队列头文件

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern UART_HandleTypeDef huart1;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
QueueHandle_t g_key_queue = NULL;   /* 原生队列句柄，g_ 前缀，初始空 */
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for KEY_Task */
osThreadId_t KEY_TaskHandle;
const osThreadAttr_t KEY_Task_attributes = {
  .name = "KEY_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);//打印
void StartTask02(void *argument);//按键扫描

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN Init */
  g_key_queue = xQueueCreate(10, sizeof(uint32_t));
  if (NULL == g_key_queue)
  {
      printf("key_queue create failed\r\n");
  }
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
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of KEY_Task */
  KEY_TaskHandle = osThreadNew(StartTask02, NULL, &KEY_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
    uint32_t received_value = 0;
  /* Infinite loop */
  for (;;)
  {
      if (NULL == g_key_queue)
      {
          vTaskDelay(pdMS_TO_TICKS(10));
          continue;
      }

      if (pdTRUE == xQueueReceive(g_key_queue, &received_value,
                                  pdMS_TO_TICKS(100)))
      {
          printf("Key pressed! msg = %lu\r\n",
                 (unsigned long)received_value);
      }
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the KEY_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
    key_event_t key_event = KEY_EVENT_NONE;
  uint32_t    msg_value = 0;
  /* Infinite loop */
for (;;)
  {
      key_event = key_scan();

      if (KEY_EVENT_PRESSED == key_event)
      {
          msg_value++;
          if (pdTRUE != xQueueSendToBack(g_key_queue, &msg_value, 0)) //判断传送队列有没有满
          {
              printf("queue send failed\r\n");
          }
      }

      vTaskDelay(pdMS_TO_TICKS(KEY_SCAN_PERIOD_MS)); 
      /* 任务休眠10ms，之后再次执行按键检测，实现软件消抖的轮询按键 */
      /* pdMS_TO_TICKS(x)，把毫秒时间转换成系统节拍tick计数值。 */
      /* vTaskDelay( ticks ) 延时函数 */
  }
  /* USER CODE END StartTask02 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */




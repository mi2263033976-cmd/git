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
#include "bsp_led.h"
#include "queue.h"  //freertos队列头文件

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern UART_HandleTypeDef huart1;

static uint8_t g_blink_cnt = 0;        /* 剩余闪烁翻转次数 */

#define KEY_LONG_BLINK_TOGGLES 6       /* 3 次亮灭 = 翻转 6 次 */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
QueueHandle_t g_key_queue = NULL;   /* 按键队列句柄，g_ 前缀，初始空 */
QueueHandle_t g_led_queue = NULL;   /* LED 队列句柄，g_ 前缀，初始空 */

osThreadId_t LedTaskHandle;         /* LED 任务句柄（手写在保留区，Cube 不会删）*/
const osThreadAttr_t LedTask_attributes = {
  .name = "LED_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE END Variables */
/* Definitions for UsartTask */
osThreadId_t UsartTaskHandle;
const osThreadAttr_t UsartTask_attributes = {
  .name = "Usart_Task",
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
void LedTask(void *argument);   /* LED 任务：消费队列消息并翻转 LED */
/* USER CODE END FunctionPrototypes */

void StartUsartTask(void *argument);//打印
void KeyTask(void *argument);//按键扫描

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN Init */
  g_key_queue = xQueueCreate(10, sizeof(key_event_t));
  if (NULL == g_key_queue)
  {
      printf("key_queue create failed\r\n");
  }

  g_led_queue = xQueueCreate(10, sizeof(key_event_t));
  if (NULL == g_led_queue)
  {
      printf("led_queue create failed\r\n");
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
  /* creation of UsartTask */
  UsartTaskHandle = osThreadNew(StartUsartTask, NULL, &UsartTask_attributes);

  /* creation of KEY_Task */
  KEY_TaskHandle = osThreadNew(KeyTask, NULL, &KEY_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  LedTaskHandle = osThreadNew(LedTask, NULL, &LedTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartUsartTask */
/**
  * @brief  Function implementing the UsartTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartUsartTask */
void StartUsartTask(void *argument)
{

  /* USER CODE BEGIN StartUsartTask */
  key_event_t key_evt = KEY_EVENT_NONE;
  /* 阻塞等待 LED 状态消息，收到后走串口打印（UART 仅由本任务独占，避免多任务打印冲突）*/
  for (;;)
  {
    if (NULL == g_led_queue)
    {
      printf("led_queue create failed\r\n");
      Error_Handler();      /* 或断言死循环——失败就别继续启动了 */
    }
    if (pdTRUE == xQueueReceive(g_led_queue, &key_evt, pdMS_TO_TICKS(100)))
    {
      switch(key_evt)
      {
        case KEY_EVENT_NONE:
          printf("KEY_EVENT_NONE\r\n");
          break;
        case KEY_EVENT_CLICK_PRESSED:
          printf("KEY_EVENT_CLICK_PRESSED\r\n");
          break;
        case KEY_EVENT_LONG_PRESSED:
          printf("KEY_EVENT_LONG_PRESSED\r\n");
          break;
        default:
          printf("KEY_EVENT_UNKNOWN\r\n");
          break;
      }
    }

      
  }
  /* USER CODE END StartUsartTask */
}

/* USER CODE BEGIN Header_KeyTask */
/**
* @brief Function implementing the KEY_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_KeyTask */
void KeyTask(void *argument)
{
  /* USER CODE BEGIN KeyTask */
  key_event_t key_event = KEY_EVENT_NONE;
  /* Infinite loop */
for (;;)
  {
    key_event = key_scan();
    if (KEY_EVENT_NONE != key_event)
    {
        /* 原生或 CMSIS 队列 API 均可，保持与工程一致 */
        xQueueSendToBack(g_key_queue, &key_event, 0);
    }
     vTaskDelay(pdMS_TO_TICKS(KEY_SCAN_PERIOD_MS)); 
      /* 任务休眠10ms，之后再次执行按键检测，实现软件消抖的轮询按键 */
      /* pdMS_TO_TICKS(x)，把毫秒时间转换成系统节拍tick计数值。 */
      /* vTaskDelay( ticks ) 延时函数 */
  }
  /* USER CODE END KeyTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
 * @brief  LED task: consumes queue messages and toggles the LED.
 * @param  argument: Not used
 * @retval None
 */
void LedTask(void *argument)
{
  for (;;)
  {
    key_event_t evt = KEY_EVENT_NONE;
    if (NULL == g_key_queue)
    {
     printf("key_queue create failed\r\n");
      Error_Handler();      /* 或断言死循环——失败就别继续启动了 */
    }
    /* 非阻塞等待按键消息， */
    if (pdTRUE == xQueueReceive(g_key_queue, &evt,0))
    {
      if (0 == g_blink_cnt)//翻转
      {
        if(KEY_EVENT_CLICK_PRESSED == evt)//点按一次
        {
          led_toggle();
          xQueueSendToBack(g_led_queue, &evt, 0);   /* 发打印指示 */
        }
        else if (KEY_EVENT_LONG_PRESSED == evt)//长按一次
        {
          g_blink_cnt = KEY_LONG_BLINK_TOGGLES;  /* 3 次亮灭 = 翻转 6 次 */
          xQueueSendToBack(g_led_queue, &evt, 0);   /* 发打印指示 */
        } 

      }
    }
    if (g_blink_cnt > 0)
    {
      led_toggle();
      g_blink_cnt--;
    }
    vTaskDelay(pdMS_TO_TICKS(200));  /* 200ms 翻转一次 */
  }
}

/* USER CODE END Application */




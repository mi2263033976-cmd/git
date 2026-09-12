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
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define KEY_LONG_PRESS_MS 800    /* 长按判定阈值(ms)：按住超过它才算长按 */
#define KEY_DEBOUNCE_MS 10
#define KEY_SCAN_MS       3      /* 按键跟踪采样周期(ms) */

/* LED 任务节拍：20ms 一次，分频实现 200ms 的闪烁翻转间隔 */
#define LED_TICK_MS       20
#define LED_BLINK_TICKS   10     /* 20ms × 10 = 200ms */

/* 事件码：KEY_Queue 里传的就是它（队列消息类型是 uint16_t，这两个值也是 uint16_t 兼容）*/
#define KEY_EVENT_CLICK_PRESSED    1
#define KEY_EVENT_LONG_PRESSED     2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static uint8_t g_key_stable = 1;   /* 已确认的稳定电平：1=高(松开)，0=低(按下) */
/* USER CODE END Variables */
/* Definitions for KEYTask */
osThreadId_t KEYTaskHandle;
const osThreadAttr_t KEYTask_attributes = {
  .name = "KEYTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for LEDTask */
osThreadId_t LEDTaskHandle;
const osThreadAttr_t LEDTask_attributes = {
  .name = "LEDTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for KEY_Queue */
osMessageQueueId_t KEY_QueueHandle;
const osMessageQueueAttr_t KEY_Queue_attributes = {
  .name = "KEY_Queue"
};
/* Definitions for KEY_Sem */
osSemaphoreId_t KEY_SemHandle;
const osSemaphoreAttr_t KEY_Sem_attributes = {
  .name = "KEY_Sem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void KEY_StartTask(void *argument);
void LED_StartTask(void *argument);

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

  /* Create the semaphores(s) */
  /* creation of KEY_Sem */
  KEY_SemHandle = osSemaphoreNew(1, 0, &KEY_Sem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of KEY_Queue */
  KEY_QueueHandle = osMessageQueueNew (16, sizeof(uint16_t), &KEY_Queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of KEYTask */
  KEYTaskHandle = osThreadNew(KEY_StartTask, NULL, &KEYTask_attributes);

  /* creation of LEDTask */
  LEDTaskHandle = osThreadNew(LED_StartTask, NULL, &LEDTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_KEY_StartTask */
/**
  * @brief  Function implementing the KEYTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_KEY_StartTask */
void KEY_StartTask(void *argument)
{
  /* USER CODE BEGIN KEY_StartTask */
  uint16_t key_evt      = 0;
  uint32_t press_tick   = 0;
  uint32_t stable_since = 0;
  uint8_t  stable_val   = 1;
  uint8_t  cur          = 0;

  for (;;)
  {
    /* ① 阻塞等中断“门铃”（没事件时任务睡着，不占 CPU）*/
    if (osOK != osSemaphoreAcquire(KEY_SemHandle, osWaitForever))
    {
      continue;
    }

    /* ② 跟踪电平：每 KEY_SCAN_MS(3ms) 采一次，直到连续稳定 KEY_DEBOUNCE_MS(10ms) */
    stable_val   = (GPIO_PIN_RESET == HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin)) ? 0 : 1;
    stable_since = osKernelGetTickCount();

    for (;;)
    {
      osDelay(KEY_SCAN_MS);
      cur = (GPIO_PIN_RESET == HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin)) ? 0 : 1;

      if (cur != stable_val)
      {
        stable_val   = cur;            /* 电平又变了 → 重新计时 */
        stable_since = osKernelGetTickCount();
        continue;
      }

      if ((osKernelGetTickCount() - stable_since) >= KEY_DEBOUNCE_MS)
      {
        break;                         /* 该电平已稳定够 → 退出跟踪 */
      }
    }

    /* ③ 用“稳定电平”驱动状态机 */
    if (0 == stable_val)               /* 稳定为“按下” → 只记时刻，不动作 */
    {
      press_tick   = osKernelGetTickCount();  /* 每次稳定按下都刷新（连按也不会误判长按）*/
      g_key_stable = 0;
    }
    else                               /* 稳定为“松开” → 此时才判定并响应 */
    {
      if (0 == g_key_stable)                 /* 之前确实按下过 */
      {
        uint32_t press_ms = osKernelGetTickCount() - press_tick;

        if (press_ms >= KEY_LONG_PRESS_MS)
        {
          key_evt = KEY_EVENT_LONG_PRESSED;  /* 长按：闪烁 3 次 */
        }
        else
        {
          key_evt = KEY_EVENT_CLICK_PRESSED; /* 单击：松开时翻转一次 */
        }

        if (osOK != osMessageQueuePut(KEY_QueueHandle, &key_evt, 0, 0))
        {
          /* 队列满：丢弃本次事件（按键事件丢失可接受）*/
        }
      }

      g_key_stable = 1;
    }
  }
  /* USER CODE END KEY_StartTask */
}

/* USER CODE BEGIN Header_LED_StartTask */
/**
* @brief Function implementing the LEDTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LED_StartTask */
void LED_StartTask(void *argument)
{
  /* USER CODE BEGIN LED_StartTask */
  uint16_t key_evt   = 0;
  uint8_t  blink_cnt = 0;          /* 剩余闪烁翻转次数 */
  uint8_t  tick_cnt  = 0;          /* 节拍分频计数 */

  for (;;)
  {
    /* 非阻塞取事件（timeout=0），不阻塞下面的闪烁节拍 */
    if (osOK == osMessageQueueGet(KEY_QueueHandle, &key_evt, NULL, 0))
    {
      if (0 == blink_cnt)          /* 空闲（未在闪烁）才响应新事件 */
      {
        if (KEY_EVENT_CLICK_PRESSED == key_evt)
        {
          HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);   /* 单击：翻转一次 */
        }
        else if (KEY_EVENT_LONG_PRESSED == key_evt)
        {
          blink_cnt = 6;           /* 长按：6 次翻转 = 闪 3 下 */
        }
      }
    }

    /* 闪烁驱动：20ms 节拍，分频到 200ms 翻转一次 */
    if (blink_cnt > 0)
    {
      tick_cnt++;
      if (tick_cnt >= LED_BLINK_TICKS)     /* 20ms × 10 = 200ms */
      {
        tick_cnt = 0;
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        blink_cnt--;
      }
    }
    else
    {
      tick_cnt = 0;
    }

    osDelay(LED_TICK_MS);        /* 20ms 节拍：事件响应更快 */
  }
  /* USER CODE END LED_StartTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/**
 * @brief  EXTI 中断回调：只做最短通知，不做判定
 * @param  GPIO_Pin: 触发中断的引脚
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (KEY_Pin == GPIO_Pin)              /* 共享回调，必须判引脚 */
    {
        osSemaphoreRelease(KEY_SemHandle);   /* 只干这一件事：按门铃 */
    }
}

/* USER CODE END Application */

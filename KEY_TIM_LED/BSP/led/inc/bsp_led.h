/******************************************************************************
 * Copyright (C) 2026 <BUBUGou>
 *
 * All Rights Reserved.
 *
 * @file bsp_led.h
 *
 * @par dependencies
 * - main.h       引脚宏: LED_Pin / LED_GPIO_Port
 *
 * @author <BUBUGou> | <班级/部门> | <学校/公司>
 *
 * @brief Provide the LED control APIs (set / get / toggle).
 *
 * Processing flow:
 *
 * call directly.
 *
 * @version V1.1 2026-09-08
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __BSP_LED_H__
#define __BSP_LED_H__

//******************************* Includes *******************************//

#include "main.h"         /* 引脚宏定义 (LED_Pin / LED_GPIO_Port) */
#include <stdint.h>       /* 编译器提供的通用库包含部分 */
#include "tim.h"           /* TIM_HandleTypeDef htim2; */
#include "freertos.h"       /* FreeRTOS API: osMessageQueueNew() / osThreadNew() / osDelay() */
#include "task.h"
#include "cmsis_os.h"      /* FreeRTOS API: osMessageQueueNew() / osThreadNew() / osDelay() */
//******************************* Includes *******************************//

//******************************** Defines **********************************//
#define LED_LONG_TOGGLE 20U
#define LED_CLICK_TOGGLE 2U

/* 命令码：由 key_task_func 判定产生，交给 led_task_func 执行 */
#define LED_CMD_CLICK   1U      /* 单击：闪 1 次 */
#define LED_CMD_LONG    2U      /* 长按：闪 10 次 */

extern osMessageQueueId_t led_queue;    /* 定义在 bsp_led.c */



/* LED 状态枚举 */
/* 命令：要做什么 + 判定依据（dt 仅供日志/调试） */
typedef struct
{
    uint16_t cmd;      /* LED_CMD_CLICK / LED_CMD_LONG */
    uint32_t dt;       /* 按下时长(ms) */
} led_cmd_t;



//******************************** Defines **********************************//

//******************************** Declaring ********************************//

void led_blink_start(uint16_t toggles);

void led_blink_tick_handler(void);

void led_task_func(void *argument);

//******************************** Declaring ********************************//

#endif /* __BSP_LED_H__ */

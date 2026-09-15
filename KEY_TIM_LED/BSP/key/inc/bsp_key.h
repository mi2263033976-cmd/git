/******************************************************************************
 * Copyright (C) 2026 <BUBUGou>
 *
 * All Rights Reserved.
 *
 * @file bsp_key.h
 *
 * @par dependencies
 * - main.h       引脚宏: KEY_Pin / KEY_GPIO_Port
 *
 * @author <BUBUGou> | <班级/部门> | <学校/公司>
 *
 * @brief Provide the polling key scan API with click / long-press detection.
 *
 * Processing flow:
 *
 * call key_scan() periodically from a task every 10ms.
 *
 * @version V1.1 2026-09-09
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

//******************************* Includes *******************************//

#include <stdint.h>       /* 编译器提供的通用库包含部分 */
#include "main.h"         /* 引脚宏定义 */
#include "cmsis_os.h"      /* FreeRTOS API: osMessageQueueNew() / osThreadNew() / osDelay() */


//******************************* Includes *******************************//

//******************************** Defines **********************************//
typedef enum
{
    KEY_EDGE_FALL = 0,          /* 按下沿(下降沿) */
    KEY_EDGE_RISE = 1,          /* 松开沿(上升沿) */
} key_edge_t;   

typedef struct
{
    key_edge_t edge;               /* 按键沿类型 */
    uint32_t    tick;          /* 该边沿发生的系统时刻（1 tick = 1ms） */
} key_event_t;

#define KEY_LONG_PRESS_MS      1000   /* 长按阈值(ms)：按下超过该值判长按 */
#define KEY_GLITCH_MS       20U     /* 小于它的"按下-松开"对视为抖动，丢弃 */

//******************************** Defines **********************************//

//******************************** Declaring ********************************//

/**
 * @brief Scans the key and reports a click / long-press event.
 *
 * Steps:
 *
 * 1. Reads the current GPIO level of the key pin.
 * 2. Counts consecutive identical samples to filter bouncing.
 * 3. On the release (rising) edge, compares the press duration with the
 *    long-press threshold and reports a CLICK or LONG_PRESS event once.
 *
 * @note This function is non-blocking and MUST be called every 10ms.
 * @note Key polarity: pull-up input, pressed = low.
 *
 * @return key_event_t : KEY_EVENT_CLICK_PRESSED / KEY_EVENT_LONG_PRESSED /
 *                       KEY_EVENT_NONE.
 *
 */
void key_task_func(void *argument);

//******************************** Declaring ********************************//

#endif /* __BSP_KEY_H__ */

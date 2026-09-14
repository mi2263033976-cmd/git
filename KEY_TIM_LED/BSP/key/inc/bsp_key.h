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

//******************************* Includes *******************************//

//******************************** Defines **********************************//

#define KEY_SCAN_PERIOD_MS      10   /* 调用周期(ms)：调用方须按此节奏调用 */
#define KEY_LONG_PRESS_MS      1000   /* 长按阈值(ms)：按下超过该值判长按 */

/* 按键事件枚举：key_scan() 每次调用的返回值 */
typedef enum
{
    KEY_EVENT_NONE          = 0,          /* 无事件：未按下、松开后非点击/长按等 */
    KEY_EVENT_CLICK_PRESSED = 1,          /* 单击事件：按下后快速松开            */
    KEY_EVENT_LONG_PRESSED  = 2,          /* 长按事件：按住超过阈值后松开        */
    KEY_EVENT_RESERVED      = 0x7FFFFFFF  /* Reserved                           */
} key_event_t;

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
key_event_t key_scan(void);

//******************************** Declaring ********************************//

#endif /* __BSP_KEY_H__ */

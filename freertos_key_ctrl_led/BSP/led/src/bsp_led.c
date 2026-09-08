/******************************************************************************
 * Copyright (C) 2026 <BUBUGou>
 *
 * All Rights Reserved.
 *
 * @file bsp_led.c
 *
 * @par dependencies
 * - bsp_led.h
 *
 * @author <BUBUGou> | <班级/部门> | <学校/公司>
 *
 * @brief Provide the polling key scan APIs.
 *
 * Processing flow:
 *
 * call key_scan() periodically from a task every 10ms.
 *
 * @version V1.0 2026-09-07
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#include "bsp_led.h"

//******************************** Defines **********************************//


//******************************** Defines **********************************//

//******************************** Static variables *************************//
static led_event_t g_led_last_state = LED_EVENT_OFF;  /* 上次确认的稳定状态 */


//******************************** Static variables *************************//

//******************************** Functions ********************************//
led_event_t led_control(led_event_t led_state)
{
    /* 1. 读取当前状态 */
    led_event_t cur_state = g_led_last_state;

    /* 2. 与稳定状态一致：无事件产生 */
    if (led_state == g_led_last_state)
    {
        return LED_EVENT_NONE;
    }
    /* 3. 状态变化：立即更新LED输出电平 */
    else
    {
        /* 4. 更新LED输出电平 */
        if (LED_EVENT_ON == led_state)
        {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        }
        else if (LED_EVENT_OFF == led_state)
        {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        }

        /* 5. 更新稳定状态 */
        g_led_last_state = led_state;

        /* 6. 返回事件类型 */
        return led_state;
    }
}


//******************************** Functions ********************************//


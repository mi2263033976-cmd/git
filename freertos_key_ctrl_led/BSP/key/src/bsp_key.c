/******************************************************************************
 * Copyright (C) 2026 <BUBUGou>
 *
 * All Rights Reserved.
 *
 * @file bsp_key.c
 *
 * @par dependencies
 * - bsp_key.h
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

#include "bsp_key.h"

//******************************** Defines **********************************//

#define KEY_DEBOUNCE_CNT_MAX   2   /* 连续2次一致才确认电平变化(~20ms) */

//******************************** Defines **********************************//

//******************************** Static variables *************************//

static GPIO_PinState g_key_last_level = GPIO_PIN_SET;  /* 上次确认的稳定电平 */
static uint8_t       g_key_db_cnt     = 0;             /* 消抖连续计数       */

//******************************** Static variables *************************//

//******************************** Functions ********************************//

key_event_t key_scan(void)    
{
    GPIO_PinState cur_level = GPIO_PIN_SET; //初始默认赋值为高电平，属于初始兜底，防止变量未初始化乱值。

    /* 1. 读取当前电平 */
    cur_level = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);

    /* 2. 与稳定电平一致：计数清零，无事件产生 */
    if (cur_level == g_key_last_level)
    {
        g_key_db_cnt = 0;
    }
    /* 3. 电平变化：需连续 KEY_DEBOUNCE_CNT_MAX 次一致才算稳定 */
    else if (++g_key_db_cnt >= KEY_DEBOUNCE_CNT_MAX)
    {
        /* 4. 仅在“松开→按下”下降沿上报一次事件 */
        if ((GPIO_PIN_SET == g_key_last_level)
         && (GPIO_PIN_RESET == cur_level))
        {
            g_key_last_level = cur_level;
            g_key_db_cnt     = 0;
            return KEY_EVENT_PRESSED;   //return 1
        }

        /* 松开沿：只更新状态，不产生事件 */
        g_key_last_level = cur_level;
        g_key_db_cnt     = 0;
    }

    return KEY_EVENT_NONE;  //return 0
}

//******************************** Functions ********************************//


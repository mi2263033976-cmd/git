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

#include "bsp_key.h"

//******************************** Defines **********************************//

#define KEY_DEBOUNCE_CNT_MAX   2   /* 连续2次一致才确认电平变化(~20ms) */

//******************************** Defines **********************************//

//******************************** Static variables *************************//

static GPIO_PinState g_key_last_level = GPIO_PIN_SET;  /* 稳定电平 */
static uint8_t       g_key_db_cnt     = 0;             /* 消抖计数 */
static uint8_t       g_press_active   = 0;             /* 1=正处于按下 */
static uint8_t       g_press_cnt      = 0;             /* 按下期间调用次数 */

//******************************** Static variables *************************//

//******************************** Functions ********************************//

key_event_t key_scan(void)    
{
    GPIO_PinState cur_level = GPIO_PIN_SET; //初始默认赋值为高电平，属于初始兜底，防止变量未初始化乱值。

    /* 1. 读取当前电平 */
    cur_level = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);

    /* 2.1 处于稳定按下状态：累加按下时长计数 */
    if (g_press_active && (GPIO_PIN_RESET == g_key_last_level))
    {
        g_press_cnt++;
    } 
    /* 2.2 与稳定电平一致：计数清零，无事件产生 */
    if (cur_level == g_key_last_level)
    {
        g_key_db_cnt = 0;
    }
    /* 3. 电平变化：需连续 KEY_DEBOUNCE_CNT_MAX 次一致才算稳定 */
    else if (++g_key_db_cnt >= KEY_DEBOUNCE_CNT_MAX)
    {
        /* 4. 按下按键 */
        if ((GPIO_PIN_SET == g_key_last_level)
         && (GPIO_PIN_RESET == cur_level))
        {
            g_press_active = 1;  /* 标记按下 */
            g_press_cnt     = 0;   /* 按下计数清零 */        
        }
        /* 5. 松开按键*/
        else if ((GPIO_PIN_RESET == g_key_last_level)
              && (GPIO_PIN_SET == cur_level))
        {
            if (g_press_active)  /* 处于按下状态才判定（防止误报）*/
            {
                uint32_t press_duration = g_press_cnt * KEY_SCAN_PERIOD_MS;  /* 按下持续时间(ms) */
                key_event_t evt = KEY_EVENT_NONE;  /* 默认无事件 */
                if (press_duration >= KEY_LONG_PRESS_MS)
                {
                    evt = KEY_EVENT_LONG_PRESSED;  /* 长按事件 */
                }
                else
                {
                    evt = KEY_EVENT_CLICK_PRESSED;  /* 单击事件 */
                }
                g_press_active = 0;
                g_press_cnt    = 0;
                g_key_last_level = cur_level;
                g_key_db_cnt     = 0;
                return evt;           
            }
        }
        /* 确认电平变化后：更新稳定电平与消抖计数（事件路径已在上面 return，不会到此）*/
        g_key_last_level = cur_level;
        g_key_db_cnt     = 0;
    }

    return KEY_EVENT_NONE;
}

//******************************** Functions ********************************//


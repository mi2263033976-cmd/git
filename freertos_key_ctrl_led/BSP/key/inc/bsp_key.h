/******************************************************************************
 * Copyright (C) 2026 <BUBUGou>
 *
 * All Rights Reserved.
 *
 * @file bsp_key.h
 *
 * @par dependencies
 * - bsp_key.h       引脚宏: KEY_Pin / KEY_GPIO_Port 
 * - stdint.h
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

#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

//******************************* Includes *******************************//

#include <stdint.h>       /* 编译器提供的通用库包含部分 */
#include "main.h"         /* 引脚宏定义 */
#include "stm32f4xx_hal.h" /* HAL库包含部分 */
#include "stm32f4xx_hal_gpio.h" /* HAL库GPIO包含部分 */

//******************************* Includes *******************************//

//******************************** Defines **********************************//

/* 按键事件枚举：key_scan() 每次调用的返回值 */
typedef enum
{
    KEY_EVENT_NONE        = 0,          /* 无事件：未按下或仍在消抖中       */
    KEY_EVENT_PRESSED     = 1,          /* 按下事件：检测到一次下降沿+消抖  */
    KEY_EVENT_RESERVED    = 0x7FFFFFFF  /* Reserved                         */
} key_event_t;

//******************************** Defines **********************************//

//******************************** Declaring ********************************//

/**
 * @brief Scans the key and reports a debounced press event.
 *
 * Steps:
 *
 * 1. Reads current GPIO level of the key pin.
 * 2. Counts consecutive identical samples to filter bouncing.
 * 3. Reports KEY_EVENT_PRESSED only on the falling edge.
 *
 * @note This function is non-blocking and MUST be called every 10ms.
 * @note Key polarity: pull-up input, pressed = low.
 *
 * @return key_event_t : KEY_EVENT_PRESSED / KEY_EVENT_NONE.
 *
 */
key_event_t key_scan(void);

//******************************** Declaring ********************************//


#endif /* __BSP_KEY_H__ */


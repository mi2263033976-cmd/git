/******************************************************************************
 * Copyright (C) 2026 <BUBUGou>
 *
 * All Rights Reserved.
 *
 * @file bsp_led.h
 *
 * @par dependencies
 * - main.h       引脚宏: LED_Pin / LED_GPIO_Port 
 * - stdint.h
 *
 * @author <BUBUGou> | <班级/部门> | <学校/公司>
 *
 * @brief Provide the polling LED control APIs.
 *
 * Processing flow:
 *
 * call led_control() periodically from a task every 10ms.
 *
 * @version V1.0 2026-09-07
 *
 * @note 1 tab == 4 spaces!
 *
 *****************************************************************************/

#ifndef __BSP_LED_H__
#define __BSP_LED_H__

//******************************* Includes *******************************//

#include <stdint.h>       /* 编译器提供的通用库包含部分 */
#include "main.h"         /* 引脚宏定义 */
//#include "stm32f4xx_hal.h" /* HAL库包含部分 */
//#include "stm32f4xx_hal_gpio.h" /* HAL库GPIO包含部分 */

//******************************* Includes *******************************//

//******************************** Defines **********************************//
/* led亮灭事件枚举 */
typedef enum
{
    LED_EVENT_OFF        = 0,          /*        */
    LED_EVENT_ON     = 1,          /*        */
    LED_EVENT_RESERVED    = 0x7FFFFFFF  /* Reserved                         */
} led_event_t;

//******************************** Defines **********************************//

//******************************** Declaring ********************************//

/**
 * @brief Controls the LED state.
 *
 * @param led_state : LED_ON / LED_OFF.
 * @retval None
 */


//******************************** Declaring ********************************//

#endif /* __BSP_LED_H__ */





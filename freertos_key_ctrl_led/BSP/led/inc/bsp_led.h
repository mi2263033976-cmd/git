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

//******************************* Includes *******************************//

//******************************** Defines **********************************//

/* LED 状态枚举 */
typedef enum
{
    LED_OFF       = 0,          /* 熄灭                          */
    LED_ON        = 1,          /* 点亮                          */
    LED_RESERVED  = 0x7FFFFFFF  /* Reserved                      */
} led_state_t;

//******************************** Defines **********************************//

//******************************** Declaring ********************************//

/**
 * @brief Reads the current LED state from the GPIO pin.
 *
 * Steps:
 *
 * 1. Reads the GPIO input level of the LED pin.
 * 2. Maps the level to LED_ON / LED_OFF with the polarity macro.
 *
 * @return led_state_t : LED_ON / LED_OFF.
 *
 */
led_state_t led_get(void);

/**
 * @brief Sets the LED to a target state.
 *
 * Steps:
 *
 * 1. Validates the input state.
 * 2. Writes the mapped GPIO level to the LED pin.
 *
 * @param[in] led_state : LED_ON / LED_OFF.
 *
 * @return led_state_t : The applied state, or current state if invalid.
 *
 */
led_state_t led_set(led_state_t led_state);

/**
 * @brief Toggles the LED and returns the new state.
 *
 * Steps:
 *
 * 1. Reads the current state.
 * 2. Writes the opposite state.
 *
 * @return led_state_t : New state after toggling (LED_ON / LED_OFF).
 *
 */
led_state_t led_toggle(void);

//******************************** Declaring ********************************//

#endif /* __BSP_LED_H__ */

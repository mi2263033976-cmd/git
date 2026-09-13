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

#include "bsp_led.h"

//******************************** Defines **********************************//

/* LED 极性宏：PC13 低电平点亮(WeAct 板载 LED)。
   若实测相反(高电平点亮)，把两行的 SET/RESET 互换即可。      */
#define LED_ON_LEVEL     GPIO_PIN_RESET
#define LED_OFF_LEVEL    GPIO_PIN_SET

//******************************** Defines **********************************//

//******************************** Functions ********************************//

led_state_t led_get(void)
{
    if (LED_ON_LEVEL == HAL_GPIO_ReadPin(LED_GPIO_Port, LED_Pin))
    {
        return LED_ON;
    }

    return LED_OFF;
}

led_state_t led_set(led_state_t led_state)
{
    if (LED_ON == led_state)
    {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, LED_ON_LEVEL);
    }
    else if (LED_OFF == led_state)
    {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, LED_OFF_LEVEL);
    }
    else
    {
        return led_get();           /* 非法参数：不改动硬件，返回当前状态 */
    }

    return led_state;
}

led_state_t led_toggle(void)
{
    if (LED_ON == led_get())
    {
        led_set(LED_OFF);
    }
    else
    {
        led_set(LED_ON);
    }

    return led_get();
}

//******************************** Functions ********************************//

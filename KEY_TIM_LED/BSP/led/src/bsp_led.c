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
#include "main.h"         /* 引脚宏定义 (LED_Pin / LED_GPIO_Port) */
#include "bsp_led.h"
#include "tim.h"           /* TIM_HandleTypeDef htim2; */
#include "freertos.h"       /* FreeRTOS API: osMessageQueueNew() / osThreadNew() / osDelay() */
#include "task.h"

//******************************** Defines **********************************//

/* LED 极性宏：PC13 低电平点亮(WeAct 板载 LED)。
   若实测相反(高电平点亮)，把两行的 SET/RESET 互换即可。      */
#define LED_ON_LEVEL     GPIO_PIN_RESET
#define LED_OFF_LEVEL    GPIO_PIN_SET

static volatile uint16_t g_blink_toggles = 0U  ;
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

void led_blink_start(uint16_t toggles)
{
    taskENTER_CRITICAL();                  /* 挡住 TIM2 中断，避免与 ISR 的递减竞争 */
    HAL_TIM_Base_Stop_IT(&htim2);          /* ① 停表 + 关更新中断 */
    g_blink_toggles = toggles;             /* ② 写入剩余翻转次数 */
    __HAL_TIM_SET_COUNTER(&htim2, 0U);     /* ③ 计数器清零：第一次翻转严格在 100ms 后 */
    HAL_TIM_Base_Start_IT(&htim2);         /* ④ 启动 */
    taskEXIT_CRITICAL();
}

void led_blink_tick_handler(void)
{
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);   /* 5Hz：每 100ms 翻转一次 */
    if (g_blink_toggles > 0U)
    {
        g_blink_toggles--;
    }
    if (0U == g_blink_toggles)
    {
        HAL_TIM_Base_Stop_IT(&htim2);             /* 闪完自停：不占中断 */
    }
}

//******************************** Functions ********************************//

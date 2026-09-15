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

extern osMessageQueueId_t LED_QueueHandle;    /* 定义在 bsp_led.c */
//******************************** Defines **********************************//

//******************************** Functions ********************************//

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

osMessageQueueId_t led_queue = NULL;      /* 与 bsp_key.c 里的 key_queue 同理 */

void led_task_func(void *argument)
{
    led_cmd_t c;
    (void)argument;

    led_queue = osMessageQueueNew(4U, sizeof(led_cmd_t), NULL);

    for (;;)
    {
        if (osOK != osMessageQueueGet(led_queue, &c, NULL, osWaitForever))
        {
            continue;
        }

        if (g_blink_toggles > 0U)          /* 正在闪 → 忽略新命令（设计决定） */
        {
            log_printf("[LED] busy -> cmd %s ignored\r\n", 
                (LED_CMD_LONG == c.cmd) ? "LONG" : "CLICK");   /* ★ 如实记录"被忽略" */
            continue;
        }

        log_printf("[LED] %s dt=%lu -> blink %u toggles\r\n",
                   (LED_CMD_LONG == c.cmd) ? "LONG" : "CLICK",
                   (unsigned long)c.dt,
                   (LED_CMD_LONG == c.cmd) ? LED_LONG_TOGGLE : LED_CLICK_TOGGLE);

        switch (c.cmd)
        {
        case LED_CMD_CLICK: led_blink_start(LED_CLICK_TOGGLE);  break;   /* 闪 1 次 */
        case LED_CMD_LONG:  led_blink_start(LED_LONG_TOGGLE);   break;   /* 闪 10 次 */
        default: break;
        }
    }
}

//******************************** Functions ********************************//

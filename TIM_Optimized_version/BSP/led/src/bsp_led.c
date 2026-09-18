#include "main.h"         /* 引脚宏定义 (LED_Pin / LED_GPIO_Port) */
#include "bsp_led.h"
#include "tim.h"          /* TIM3 句柄 htim3 */

//******************************** Defines **********************************//

static volatile uint16_t g_pwm_cycles = 0U  ;

//******************************** Defines **********************************//

//******************************** Functions ********************************//
void led_pwm_blink_start(uint16_t cycles)
{
    taskENTER_CRITICAL();                                  /* 挡住更新中断，防竞争 */
    __HAL_TIM_DISABLE_IT(&htim3, TIM_IT_UPDATE);           /* ① 先关计数中断 */
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);               /* ② 停 PWM 输出 */
    g_pwm_cycles = cycles;                                 /* ③ 写入剩余周期数 */
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, LED_PWM_DUTY);  /* ④ 恢复 50% 占空比 */
    __HAL_TIM_SET_COUNTER(&htim3, 0U);                     /* ⑤ 计数器清零：从完整周期开始 */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);              /* ⑥ 起 PWM */
    __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);            /* ⑦ 开更新中断（每周期一次） */
    taskEXIT_CRITICAL();
}

/* 由 HAL_TIM_PeriodElapsedCallback 的 TIM3 分支调用（更新中断里，5Hz） */
void led_pwm_tick_handler(void)
{
    if (g_pwm_cycles > 0U)
    {
        g_pwm_cycles--;
    }
    if (0U == g_pwm_cycles)
    {
        __HAL_TIM_DISABLE_IT(&htim3, TIM_IT_UPDATE);
        /* 反相接法：拉高才灭 → 占空比 100%（恒高） */
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, LED_PWM_PERIOD);
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);            /* 再关输出 */
    }
}

led_status_t led_init(void)//（建队列并返回状态）
{
    led_queue = xQueueCreate(4U, sizeof(led_cmd_t));
    return (NULL == led_queue) ? LED_ERRORRESOURCE : LED_OK;
}

QueueHandle_t led_queue = NULL;

void led_task_func(void *argument)//（阻塞等命令 → 判忙 → 起闪）
{
    led_cmd_t c;
    (void) argument;
	//测试用
	led_cmd_t t1 = { .cmd = LED_CMD_LONG,  .dt = 0U };
	led_cmd_t t2 = { .cmd = LED_CMD_CLICK, .dt = 0U };
	(void)xQueueSendToBack(led_queue, &t1, 0U);
	(void)xQueueSendToBack(led_queue, &t2, 0U);
	
	for(;;)
	{
		if (pdTRUE != xQueueReceive(led_queue, &c, portMAX_DELAY))  /* 阻塞等命令 */
		{
			continue;
		}
        if (g_pwm_cycles > 0U) 
        { 
            continue;
        }   /* 还在闪 → 丢掉这条，回去等新命令 */
		if (LED_CMD_CLICK == c.cmd)
		{
			led_pwm_blink_start(LED_CLICK_CYCLES);
		}
		else if (LED_CMD_LONG == c.cmd)
		{
			led_pwm_blink_start(LED_LONG_CYCLES);
		}
	}
}
//******************************** Functions ********************************//

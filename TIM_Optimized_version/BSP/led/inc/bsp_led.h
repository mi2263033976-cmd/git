#ifndef __BSP_LED_H
#define __BSP_LED_H
//******************************* Includes *******************************//

#include "main.h"         /* 引脚宏定义 (LED_Pin / LED_GPIO_Port) */
#include "tim.h"          /* TIM3 句柄 htim3 */
#include "FreeRTOS.h"    /* ← 必须第一个 */
#include "task.h"        /* taskENTER_CRITICAL */
#include "queue.h"       /* QueueHandle_t */
#include "bsp_log.h"    /* LOG_I() */

//******************************* Includes *******************************//

//******************************** Defines **********************************//

#define LED_PWM_PERIOD    2000U   /* = ARR+1 */
#define LED_PWM_DUTY      1000U   /* = CCR，50% */
#define LED_CLICK_CYCLES  1U  //单机翻转1次
#define LED_LONG_CYCLES   10U //长按翻转10次  

#define LED_CMD_CLICK  1U      /* 单击：闪 1 次 */
#define LED_CMD_LONG   2U      /* 长按：闪 10 次 */
typedef struct
{
    uint32_t dt;               /* 按下时长(ms)，现在只给日志留用 */
    uint16_t cmd;        /* LED_CMD_CLICK / LED_CMD_LONG */

} led_cmd_t;

typedef enum                   /* v2 风格：失败能说出为什么 */
{
    LED_OK = 0, LED_ERROR, LED_ERRORRESOURCE, LED_ERRORPARAMETER
} led_status_t;


extern QueueHandle_t led_queue;    /* key 模块要往这里发命令 */
//******************************** Defines **********************************//

//******************************** Declaring ********************************//

void led_pwm_blink_start(uint16_t cycles);
void led_pwm_tick_handler(void);
led_status_t led_init(void);
void led_task_func(void *argument);



//******************************** Declaring ********************************//
#endif


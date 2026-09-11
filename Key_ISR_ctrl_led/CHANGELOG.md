# Changelog

本项目所有值得记录的变更都会写在这个文件里。
格式参考 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)，
版本语义遵循 [SemVer](https://semver.org/lang/zh-CN/)。

## [Unreleased]

### 新增
- 初始化 STM32F411CEU6 工程：CubeMX 配置 + Keil MDK-ARM 工程
- FreeRTOS（CMSIS-RTOS v2）创建任务与队列：KEYTask / LEDTask / KEY_Queue
- 按键改用 EXTI 中断方式：PA0 双沿触发 + 二值信号量 KEY_Sem（中断只通知，判定在任务）

### 中断版实现步骤

- [x] Step 0：CubeMX 补配置（PA0 User Label=KEY、加二值信号量 KEY_Sem）+ 重新生成代码
- [ ] Step 1：核对生成结果（main.h 的 KEY_Pin/KEY_GPIO_Port、freertos.c 的 KEY_SemHandle、gpio.c 的 EXTI 双沿 + NVIC 优先级 5），并把信号量初值改为 0
- [ ] Step 2：写中断回调 HAL_GPIO_EXTI_Callback（ISR 里只 osSemaphoreRelease，不判定）
- [ ] Step 3：写 KeyTask（等信号量 → 20ms 消抖 → 判定单击/长按 → 事件发到 KEY_Queue）
- [ ] Step 4：写 LEDTask（收事件 → 单击翻转 / 长按闪烁 3 次）
- [ ] Step 5：编译 → 烧录 → 验证（单击翻一次 / 长按闪 3 下）
- [ ] Step 6：自查清单（ISR 只通知、信号量初值为 0、队列消息类型 uint16_t 一致等）

 

  
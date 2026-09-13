# Changelog

本项目所有值得记录的变更都会写在这个文件里。
格式参考 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)，
版本语义遵循 [SemVer](https://semver.org/lang/zh-CN/)。

> 来源：课程任务 12 —— 基于中断实现单击、长按的按键，硬件定时器 TIMx 精确控制 LED。
> 方案全文见 Obsidian：`FreeRTOS/学习任务/硬件定时器版按键中断-TIMx精确控制LED-方案与知识点.md`

## [Unreleased]

### 新增

- 初始化 STM32F411CEU6 工程（CubeMX + Keil MDK-ARM）：`KEY_TIM_LED`
- CubeMX 配置：
  - 时钟：HSE 25MHz 晶振 → PLL（M=12 / N=96 / P=2）→ **SYSCLK = 100MHz**；APB1 分频 2（APB1 Timer clocks = 100MHz）
  - GPIO：**PC13 = LED 输出**（初始高电平 = 灭，低电平点亮）；**PA0 = GPIO_EXTI**（下降沿触发 + 上拉，按下为低）
  - **TIM2**：PSC = 9999、ARR = 999 → 更新中断 **100ms**（10Hz），NVIC 抢占优先级 5
  - USART1：Asynchronous，115200 8N1（PA9/PA10，验证用）
  - FreeRTOS：CMSIS-RTOS v2；任务 `KEYTask`(AboveNormal) / `LEDTask`(Normal)；队列 `KEY_Queue` / `LED_Queue`
  - NVIC：EXTI0 抢占优先级 5；Priority Group = 4 bits pre-emption（默认）
  - **HAL 时基改用 TIM1**（SysTick 留给 FreeRTOS）
- 构建产物管理：`.gitignore`（Keil 编译产物 / JLink 日志 / Keil 个人设置）

### 实现步骤（计划）

- [ ] Step 0：CubeMX 建工程（Toolchain = MDK-ARM V5）→ 编译下载，确认 LED 初始灭
- [ ] Step 1：串口打通（USB-TTL 接 PA9/PA10），`log_printf()` 输出 `boot`
- [ ] Step 2：定义结构体事件 `key_evt_t { tick, edge }` 与 `LED_Queue`，编译确认 `sizeof` 生效
- [ ] Step 3：写 EXTI 回调（记 tick + 切触发沿 + 发队列），先用串口验证 FALL/RISE 两条日志
- [ ] Step 4：写 `KEYTask`（t2−t1 判单击/长按；`dt < 20ms` 毛刺丢弃），验证日志里 dt 与判定
- [ ] Step 5：写 TIM2 中断处理 `led_blink_tick_handler()`（放在 `main.c` 的 `USER CODE BEGIN Callback 1` 调用）+ 临界区版 `led_blink_start()`
- [ ] Step 6：写 `LEDTask`（收命令 → 启动 TIM2），验证短按闪 1 次、长按闪 10 次
- [ ] Step 7：全量验证 + 边界测试（快速连按 / 按住不放 / 闪烁中再按）
- [ ] Step 8：git 提交

### 设计要点（与中断版旧工程 `Key_ISR_ctrl_led` 的区别）

- 按键 EXTI 改为「**单沿 + ISR 内切沿**」：按下触发下降沿后立即切成上升沿（抖动期间无沿可触发，天然免抖），松开触发上升沿后再切回下降沿
- 中断里记录 **systickcount 时刻**，事件以结构体 `{ tick, edge }` 经队列交给任务判定（学习目标：结构体 + 队列）
- LED 闪烁由 **TIM2 更新中断（100ms/次）硬件驱动**，任务不参与闪烁节拍 → 精确 **5Hz**（100ms 翻转一次电平，200ms 一个周期）
- 单击 = 闪 1 次（2 次翻转，200ms 回到原状态）；长按（≥1000ms）= 闪 10 次（20 次翻转，约 2s）
- 验证不做示波器/逻辑分析仪实测，改用串口日志 + 肉眼/秒表

### 已知局限

- 串口验证需外接 USB-TTL 模块（板上 Type-C 直连 MCU，无 USB-TTL 芯片）
- 闪烁期间的新按键按设计被忽略（不打断当前闪烁）

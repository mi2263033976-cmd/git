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

> **代码组织（已定）**：BSP 分层 —— `BSP/key`（按键中断版）、`BSP/led`（LED + TIM2 中断处理），队列由 BSP 模块**自建**（绕开 CubeMX 队列元素类型 `sizeof` 的手改维护问题）；CubeMX 生成的任务入口 `Key_task` / `Ledtask` 只做转调。

- [x] Step 0：Keil 把 `BSP/key`、`BSP/led` 加入工程 + 两行 include 路径；编译（0 error）下载确认 LED 初始灭
- [x] Step 1：串口打通（USB-TTL 接 PA9/PA10），`log_printf()` 输出 `boot`
- [x] Step 2：`BSP/led` 补 `led_blink_start()`（临界区版）+ `led_blink_tick_handler()`；`main.c` 的 `USER CODE BEGIN Callback 1` 挂 TIM2 分支
- [x] Step 3：`BSP/key` 定义 `key_event_t { edge, tick }` + 切沿宏 + `HAL_GPIO_EXTI_Callback()`（记 tick / 切沿 / 发队列），串口验证 FALL/RISE ✅
- [x] Step 4：`key_task_func()`：`t2−t1` 判单击/长按；`dt < 20ms` 毛刺丢弃；串口验证 `dt -> CLICK/LONG` ✅
- [x] Step 5：`led_task_func()` 收命令（`led_cmd_t { cmd, dt }`）→ `led_blink_start()` 启动 TIM2；验证短按闪 1 次、长按闪 10 次 ✅
- [x] Step 6：全量验证 + 边界测试（快速连按 / 按住不放 / 闪烁中再按）+ git 提交 ✅

### 实测日志（Step 6 验收留档）

```
boot
[LED] CLICK dt=1xx -> blink 2 toggles       ← 轻点一下：灯闪 1 次（亮→灭 ≈200ms）
[LED] LONG  dt=15xx -> blink 20 toggles     ← 按住 1.5s 后松开：灯连闪 10 次（≈2s）
[LED] busy -> cmd CLICK ignored             ← 闪烁期间再按：灯不受影响（按设计忽略）
```

- 上电只出 `boot`，灯不闪；
- 按住 3s 不放 → 串口无输出、灯不动，**松手那一刻**才连闪 10 次；
- 快速连按 5 次 → 每次都闪 1 次，不丢键；
- 全部与预期一致，无异常现象。

### 设计要点（与中断版旧工程 `Key_ISR_ctrl_led` 的区别）

- 按键 EXTI 改为「**单沿 + ISR 内切沿**」：按下触发下降沿后立即切成上升沿（抖动期间无沿可触发，天然免抖），松开触发上升沿后再切回下降沿
- 中断里记录 **systickcount 时刻**，事件以结构体 `{ tick, edge }` 经队列交给任务判定（学习目标：结构体 + 队列）
- LED 闪烁由 **TIM2 更新中断（100ms/次）硬件驱动**，任务不参与闪烁节拍 → 精确 **5Hz**（100ms 翻转一次电平，200ms 一个周期）
- 单击 = 闪 1 次（2 次翻转，200ms 回到原状态）；长按（≥1000ms）= 闪 10 次（20 次翻转，约 2s）
- 验证不做示波器/逻辑分析仪实测，改用串口日志 + 肉眼/秒表

### 已知局限

- 串口验证需外接 USB-TTL 模块（板上 Type-C 直连 MCU，无 USB-TTL 芯片）
- 闪烁期间的新按键按设计被忽略（不打断当前闪烁）
- **日志归口（设计决定）**：命令的执行结果只由 LED 侧输出 —— 空闲时 `[LED] CLICK dt=120 -> blink 2 toggles`，忙时 `[LED] busy -> cmd CLICK ignored`；按键侧不再打印判定结果，避免出现"检测成功但实际被忽略"的误导日志

### 待优化（工程完成后再做，向企业级靠拢）

> 本次先"跑通优先"（用户定：**完成工程后再回头优化**）。以下为已识别的优化项，按性价比排序：

1. **日志**：`log_printf()` 加 `osMutex`（防多任务日志交叉）+ `LOG_ENABLE` 编译开关；进一步可做 `UartTask` + 队列独占串口，或环形缓冲 + `HAL_UART_Transmit_DMA`
2. **分层规范**：BSP 模块补状态枚举（`KEY_OK` / `LED_OK` …）、统一 doxygen 注释模板、函数命名规范（向参考工程 `EternalChip` 风格靠拢）
3. **健壮性**：启动顺序竞态（队列改到 init 阶段创建、ISR 里去掉 `NULL` 判断）、错误返回值不再一律 `(void)` 吞掉、关键参数校验/断言
4. **配置一致性**：RTOS 对象统一"在 CubeMX 里配"或"在代码里建"，二选一（目前 CubeMX 生成的 `KEY_Queue` / `LED_Queue` 已闲置）
5. **工程化**：README 补构建/烧录说明、静态检查（Cppcheck）、版本号与 CHANGELOG 规范

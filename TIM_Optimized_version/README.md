# KEY_TIM_LED v2 · 按键中断 + 硬件 PWM 驱动 LED（优化版）

> ⚠️ 工程目录名是 **`TIM_Optimized_version`**（不是 `KEY_TIM_LED_v2`）。
> 这是 v1 的**企业级重做版**：同一个功能，换一套"资源怎么建、LED 怎么驱动、API 怎么调"的做法。
> v1（CMSIS-RTOS 封装版）在同级目录 `../KEY_TIM_LED`，两者可并排对照。

---

## 一、功能

| 操作 | 现象 |
|---|---|
| **单击**用户按键 | 板载蓝灯闪 **1** 次 |
| **长按**（≥ 1000 ms） | 连闪 **10** 次 |
| **闪烁期间再按键** | 忽略（正在闪时不打断） |
| **上电** | 灯**不亮**（等按键命令） |

串口会同步输出分级日志，形如：

```
[001234ms][INF][KEY] CLICK dt=127
[001235ms][INF][LED] CLICK dt=127 -> blink 1
[002700ms][INF][LED] busy -> cmd CLICK ignored
```

---

## 二、硬件与接线（⚠️ 这一段务必照做）

| 项 | 说明 |
|---|---|
| 开发板 | WeAct **STM32F411CEU6** 核心板 |
| 用户按键 | **PA0**（按下 = 低电平，内部上拉），EXTI0 **下降沿**触发 |
| PWM 输出引脚 | **PA6 = TIM3_CH1**（PC13 没有任何定时器通道，所以 PWM 只能从 PA6 出） |
| **必须的接线** | **`PA6 ──杜邦线── PC13`** —— 板上 LED 回路里已有 1.5K 限流，**不需要外接电阻** |
| **PC13 配置** | **GPIO_Input + Pull-up** ⚠️ **绝不能配成输出** —— 否则与 PA6 的推挽输出**对打**（短路、可能烧脚） |
| 极性 | **反相**：PA6 输出**低** → 灯亮。所以"让灯灭"要设 `CCR = ARR`（输出恒高），不是 `CCR = 0` |
| 串口 | USART1 **115200 8N1**，接 **PA9(TX) / PA10(RX)**；板载没有 USB-TTL 芯片，需外接模块 |
| 调试器 | J-Link / ST-Link（SWD：PA13/PA14） |

> ### ⚠️ 操作顺序（反过来有烧脚风险）
> **① 先烧录程序（此时确认 PC13 已配置为输入）→ ② 再接杜邦线 `PA6—PC13`。**

---

## 三、编译

### 方式 A：Keil MDK（图形界面）
打开 `MDK-ARM/TIM_Optimized_version.uvprojx` → `F7` 编译 → `0 Error / 0 Warning`。

### 方式 B：命令行（CI / 脚本友好）

```bash
"D:/Keil_v5/UV4/UV4.exe" -r "MDK-ARM/TIM_Optimized_version.uvprojx" -j0 -o build.log
```

- **必须用 `-r`（全量重编）**：`-b` 增量编译会跳过没改动的文件、把警告藏起来
- `UV4` 退出码：`0` = 无错无警，`1` = 有警告，`2` = 有错误
- ⚠️ Keil 生成的 `build.log` 是 **UTF-16LE**，直接 `cat` 是乱码，用 `tr -d '\000' < build.log` 查看

---

## 四、烧录与验证

1. 用 J-Link/ST-Link 烧录 `.axf` / `.hex`
2. 打开串口助手（**115200 8N1**）→ 上电应看到 `BOOT` 与一条 `[000000ms][INF][SYS] n=7`（上电自检日志）
3. **再接上 `PA6—PC13` 杜邦线**
4. 按键：单击 → 灯闪 1 次；长按 → 闪 10 次；闪烁中再按 → 串口出现 `busy -> cmd ... ignored`

---

## 五、软件结构

```
TIM_Optimized_version/
├── BSP/                     ← 板级驱动（自己写的，与 CubeMX 无关）
│   ├── key/  inc|src/bsp_key.{h,c}    按键：EXTI 切沿事件 → key_queue
│   ├── led/  inc|src/bsp_led.{h,c}    LED：PWM 启动/停止/计数
│   └── log/  inc|src/bsp_log.{h,c}    日志：分级 + 队列 + 独占串口任务
├── Core/                    ← CubeMX 生成（用户代码只写在 USER CODE 区内）
│   ├── Inc/   main.h · FreeRTOSConfig.h · tim.h · usart.h ...
│   └── Src/   main.c · freertos.c（RTOS 对象与任务都在这）
├── Drivers/  Middlewares/   ← ST 的 HAL 库与 FreeRTOS 源码（第三方）
├── MDK-ARM/                 ← Keil 工程与编译产物
└── TIM_Optimized_version.ioc  ← CubeMX 配置（配置的唯一权威来源）
```

**数据流**：

```
PA0 按下/松开
   ↓ 硬件 EXTI
HAL_GPIO_EXTI_Callback()          ← 切沿免抖（FTSR/RTSR 二选一）+ 取 tick
   ↓ xQueueSendToBackFromISR
key_queue
   ↓ KEYTask（状态机：按下记 t1、松开算 dt → CLICK / LONG）
led_queue
   ↓ LEDTask（判忙 → led_pwm_blink_start）
TIM3_CH1（PA6）输出 5Hz / 50% PWM ──杜邦线──→ PC13 → 板载蓝灯
   ↑ TIM3 更新中断（5Hz）→ led_pwm_tick_handler() 递减计数，到 0 熄灯
```

---

## 六、关键设计（与 v1 的差异）

| 方面 | v1（`../KEY_TIM_LED`） | **v2（本工程）** |
|---|---|---|
| **RTOS API** | CMSIS-RTOS2 封装（`osMessageQueueNew` / `osThreadNew`） | **原生 FreeRTOS API**（`xQueueCreate` / `xTaskCreate` / `...FromISR`） |
| **ISR 里发队列** | `osMessageQueuePut`（内部自动分流 FromISR + 自动让出） | `xQueueSendToBackFromISR` + **`portYIELD_FROM_ISR`**（自己扛） |
| **对象创建时机** | 在任务里创建（靠 `if (NULL != q)` 兜竞态） | **`MX_FREERTOS_Init()` 里创建**（调度器启动前就绪，无需判空） |
| **LED 驱动** | TIM2 中断里翻 GPIO（PC13 输出） | **TIM3 硬件 PWM** 经 PA6 → 杜邦线 → PC13（PC13 为输入） |
| **日志** | `log_printf` 阻塞式（≈8ms/行）+ 多任务直接写串口（可能交叉） | **专用 LogTask 独占串口**，调用者只入队（≈µs），可分级 |
| **内存分配** | 动态（`xQueueCreate` / `xTaskCreate` 走 FreeRTOS 堆） | **全静态**（`...Static` 版），运行期零动态分配 |
| **RAM 占用（ZI）** | — | **9108 B**（省掉了 15360 B 的 FreeRTOS 堆） |
| **CMSIS 依赖** | 必须（`cmsis_os2.c`） | **无**（`cmsis_os2.c` 已移出工程） |

---

## 七、踩过的坑（改这个工程前先看这节）

1. **静态栈数组必须 8 字节对齐**
   `xTaskCreateStatic` → `prvInitialiseNewTask()` 里有
   `configASSERT((pxStack & portBYTE_ALIGNMENT_MASK) == 0)`，而 `portBYTE_ALIGNMENT = 8`。
   动态分配时 `pvPortMalloc` 会自动对齐，**静态分配就得自己声明**：
   `__align(8) static StackType_t xxxTaskStack[N];`（GCC 用 `__attribute__((aligned(8)))`）。
   不对齐的后果：`configASSERT` = `关中断 + for(;;)` → **当场卡死，且编译器不报任何错**。

2. **移除 `cmsis_os2.c` 时，别忘了它提供的 `SysTick_Handler`**
   那个文件不只是"CMSIS 适配层"——它还定义了 `SysTick_Handler`（内部调 `xPortSysTickHandler()`，即 FreeRTOS 的心跳）。
   移除后必须**自己补**，否则 SysTick 中断会回落到启动文件的 `Default_Handler`（`B .` 死循环）→ 整机卡死。
   **判据**：`.map` 里 `SysTick_Handler` 的代码只有 **2 字节**、且落在 `startup_*.o` 里，就是这个问题。

3. **ISR 里绝不能用任务版 API**
   `xQueueSendToBack`（任务版）内部第一句就是 `taskENTER_CRITICAL()`，而 `vPortEnterCritical()` 里有
   `configASSERT((ICSR & VECTACTIVE) == 0)` —— 在中断里**必然失败**。
   必须用 `xQueueSendToBackFromISR(..., &woken)` + `portYIELD_FROM_ISR(woken)`。

4. **CubeMX 里"配了 EXTI 引脚" ≠ "使能了中断"**
   `PA0 = GPIO_EXTI` 只是**引脚层**；还要在 CubeMX 的 **NVIC 页勾上 `EXTI line0 interrupt`**（优先级填 **5**），
   否则不会生成 `EXTI0_IRQHandler`、也没有 `HAL_NVIC_EnableIRQ(EXTI0_IRQn)` → 中断永不被 CPU 响应。
   **自检**：`EXTI->IMR` 有对应位，但 `NVIC->ISER[0]` 的对应位为 0 → 就是这一层的问题。

5. **能调 `...FromISR` 的中断，抢占优先级数值必须 ≥ `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY`（本工程 = 5）**
   填 0~4 会在 ISR 里踩 `vPortValidateInterruptPriority` 的 `configASSERT`。

6. **`HAL_TIM_PWM_Stop_IT()` 关的是 CC 中断，关不掉更新中断**
   要停"计数"必须 `__HAL_TIM_DISABLE_IT(&htim3, TIM_IT_UPDATE)`。

7. **CubeMX 重新 GENERATE 会重写 `.uvprojx`**
   你在 Keil 里手动挂的 `BSP` 文件组与 include 路径会丢；但 `USER CODE` 区与 `Core/` 之外的 `BSP/` 不受影响。
   **Generate 前先记好这两处设置**（Group + Include Paths）。

8. **行尾陷阱**：CubeMX 生成的文件是 LF，而历史入库版本是 CRLF，导致每次 Generate 后 `git status`
   冒出一千多个"已修改"（其实内容一字未变）。本工程已用 `.gitattributes`（`* text=auto`）统一为 LF。
   验真假：`git diff --stat --ignore-cr-at-eol -- .`

---

## 八、已知限制 / 后续可做

| 项 | 说明 |
|---|---|
| 日志用阻塞发送 | `LogTask` 里是 `HAL_UART_Transmit`（阻塞）。日志量一大就会拖慢输出；真要高频再上 DMA + 环形缓冲 |
| 日志不支持 ISR 内调用 | `log_post` 用的是任务版 `xQueueSendToBack`；要在中断里打日志需另加 `log_post_from_isr()` |
| 长按判定依赖 tick | `dt` 精度 = 1 tick（1ms），够用；要更准可用 DWT 计数器 |
| 未上 Cppcheck | 本机未安装；装好后可跑：<br>`cppcheck --enable=all --std=c99 --suppress=missingIncludeSystem -I Core/Inc -I BSP/key/inc -I BSP/led/inc -I BSP/log/inc Core/Src BSP` |
| 呼吸灯（进阶） | PWM 频率提到 ≥1kHz，用任务循环调 `CCR` 即可（当前 5Hz 是给人眼数闪烁用的） |

---

## 九、相关文档（Obsidian）

- 任务流程（做什么 / 为什么）：`FreeRTOS/学习任务/TIMx精确控制LED/优化版/v2-任务流程.md`
- 代码精讲（逐段实现）：`.../v2-代码精讲.md`
- 交接与状态（进度 / 踩坑 / 原生 API 复盘清单）：`.../00-交接与状态.md`
- v1 复盘：`.../学习总结.md`、`.../代码精讲.md`

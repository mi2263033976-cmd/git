# Changelog

本项目所有值得记录的变更都会写在这个文件里。
格式参考 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)，
版本语义遵循 [SemVer](https://semver.org/lang/zh-CN/)。

## [Unreleased]

### 新增
- 按键驱动 `BSP/key/` 完成消抖 + 下降沿检测（一次按下只上报一次事件）
- 新增 `LedTask`：消费按键队列消息并翻转 LED（按一次翻一次）
- 新增 `UartTask`：独立接收 LED 状态并串口打印，UART 由该任务独占
- LED 初始电平改为熄灭状态（PC13 上电不亮）

### 计划 / 待做
- （按需继续补充）

## [0.1.0] - 2026-09-06

### 新增
- 初始化 STM32F411CEU6 工程：CubeMX 配置 + Keil MDK-ARM 工程
- 集成 FreeRTOS（CMSIS-RTOS v2），创建 defaultTask 任务
- 初始化 GPIO（按键 / 指示灯引脚）
- 新增 BSP 按键驱动框架：`BSP/key/`（bsp_key.c / bsp_key.h）
- 纳入 git 版本管理并上传 GitHub

## [0.1.1] - 2026-09-08

### 新增
- 完善`BSP/key/`（bsp_key.c / bsp_key.h）
- 调用freertos队列实现按键与串口的调试
- 新增 BSP led灯亮灭框架：`BSP/led/`（bsp_led.c / bsp_led.h）
 

  
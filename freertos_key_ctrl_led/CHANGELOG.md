# Changelog

本项目所有值得记录的变更都会写在这个文件里。
格式参考 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)，
版本语义遵循 [SemVer](https://semver.org/lang/zh-CN/)。

## [Unreleased]

### 计划 / 待做
- 完成按键检测逻辑，实现"按键控制 LED"功能
- 补充 BSP 按键驱动的实质功能（当前仅为框架）
- （按需继续补充）

## [0.1.0] - 2026-09-06

### 新增
- 初始化 STM32F411CEU6 工程：CubeMX 配置 + Keil MDK-ARM 工程
- 集成 FreeRTOS（CMSIS-RTOS v2），创建 defaultTask 任务
- 初始化 GPIO（按键 / 指示灯引脚）
- 新增 BSP 按键驱动框架：`BSP/key/`（bsp_key.c / bsp_key.h）
- 纳入 git 版本管理并上传 GitHub

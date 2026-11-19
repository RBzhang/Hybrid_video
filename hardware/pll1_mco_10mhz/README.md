# STM32U575 PLL1-MCO 10 MHz 移频时钟

## 最新：纯 Keil MDK5 可打开的最小输出工程

**无需 STM32CubeMX，无需 HAL/LL。**

- [Keil5 最小工程和构建/测量步骤](firmware/README.md)
- [Keil 工程文件 .uvprojx](firmware/MDK-ARM/NUCLEO_U575_TagClock.uvprojx)
- [修改 TAG_ID=0..9 的唯一配置文件](firmware/Core/Inc/tag_clock_config.h)
- [PLL1/MCO 寄存器初始化代码](firmware/Core/Src/tag_clock.c)

默认内部 HSI16，不需要补焊 HSE 晶体。只输出 PA8/MCO 约 10～10.216 MHz 的数字时钟；CPU 保持复位默认 MSIS 时钟。由于内部 RC 精度有限，理论中心频率非实测保证。**尚未在实体板上编译、烧录与示波器验证。**

## 原始详细设计（多节点系统方案，历史版本）

- [完整设计文档](Design_PLL1_MCO_ZH.md)
- [PLL1-MCO 连接示意图](schematics/01_pll1_mco_shift.svg)
- [早期初始化骨架](pll1_mco_config.c)
- [FRACN 理论频率表](tag_frequency_table.csv)

旧版文档包括 HSE、1.8 V 缓冲器、ADG902 与图像链路讨论；**不代表本目录最新的纯 Keil 最小实验工程配置**。实际 Keil 构建步骤以 [firmware/README.md](firmware/README.md) 为准。

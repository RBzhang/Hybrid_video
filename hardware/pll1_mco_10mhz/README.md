# STM32U575 PLL1-MCO 10 MHz 移频时钟

**推荐简化方案：** 直接使用 STM32U575 PLL1 的 13 bit FRACN 分数配置，PLL1_R 经 MCO /8 输出 10.000～10.216 MHz，按 24 kHz 间隔分配 10 个 Tag；不需要外置 LC VCO、频率调节 DAC、÷16 测频器或 Tag 本地 FLL。

## 中文设计文档

- [完整中文设计文档（Markdown）](Design_PLL1_MCO_ZH.md)
- [完整中文设计文档（HTML）](Design_PLL1_MCO_ZH.html)
- [标准电路符号 SVG 连接图](schematics/01_pll1_mco_shift.svg)
- [PLL1/MCO 初始化代码骨架](pll1_mco_config.c)
- [10 个 Tag 的 FRACN 频率表（CSV）](tag_frequency_table.csv)

## 最终建议参数

- HSE：16 MHz 晶体；
- PLL1：M=2、N=20、R=2，PLL输入 8 MHz；
- FRACN：0～3539；
- PLL1_R：80.000～81.728 MHz；
- MCO：PLL1_R /8，PA8 / AF0；
- FRACN 频率步进：约 61.035 Hz；
- Tag 频点：10.000 MHz + TagID × 24 kHz；
- PA8 3.3 V 输出经 1.8 V 供电的 74AUP1G17 缓冲，再驱动 ADG902 CTRL。

本版本采用 M=2、N=20，较 M=1、N=10 的 122.07 Hz 步进提高一倍频率设置分辨率。10 个 FRACN 值见 CSV。

## 硬件注意事项

NUCLEO-U575ZI-Q C05 原理图把外部 HSE X3 及 C59/C60 列为可选未装，先检查实板。若没有 16 MHz HSE，必须安装合适晶体或外部 16 MHz 时钟。ST-LINK MCO 可用于单板 bring-up，但最终多 Tag 应使用稳定度明确的 HSE 参考。

本方案推荐 SYSCLK 共享 PLL1_R，因此 CPU/HCLK 随 Tag 频点变化为 80.000～81.728 MHz。需要重新核算 DCMI、FIR、DAC、定时器和 UART 时序。

3.3 V PA8 不能直接驱动 1.8 V 供电的 ADG902 CTRL。默认使用输入耐受 3.6 V 的 74AUP1G17，VCC=1.8 V。若整套 MCU I/O 与 ADG902 均为 1.8 V，才可另行评估直连。

## 设计状态

这是设计与验证方案，不是已测量通过的硬件。PLL fractional spur、24 kHz 邻道相位噪声、HSE 温漂、多 Tag 同时发送性能必须实测。完整的 2.4 GHz 图像变容调制与天线匹配网络不属于本目录的已完成范围。


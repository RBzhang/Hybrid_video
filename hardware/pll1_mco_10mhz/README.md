# STM32U575 PLL1-MCO 10 MHz 移频时钟

**推荐简化方案：** 直接使用STM32U575 PLL1的13bit FRACN分数配置，PLL1_R经MCO /8输出10.000～10.216MHz，按24kHz间隔分配10个Tag；不需要外置LC VCO和Tag本地FLL。

- [完整中文设计文档](Design_PLL1_MCO_ZH.md)
- [标准电路符号SVG连接图](schematics/01_pll1_mco_shift.svg)
- [PLL1/MCO初始化代码骨架](pll1_mco_config.c)
- [10个Tag的FRACN频率表](tag_frequency_table.csv)

**重要：** NUCLEO-U575ZI-Q C05原理图把外部HSE X3及C59/C60列为可选未装，先检查实板。若没有16MHz HSE，必须安装晶体或外部16MHz时钟。STLINK MCO 8MHz只建议临时验证，不用于最终多Tag频率精度。

本方案的SYSCLK随Tag频点为80～81.728MHz；PA8输出MCO为10～10.216MHz。3.3V PA8不能直接驱动1.8V供电的ADG902 CTRL，默认需要1.8V供电、输入耐受3.6V的74AUP1G17缓冲器；若整套MCU I/O和ADG902均为1.8V，可另行评估直连。

本文件是设计与验证计划，不是已测量通过的硬件。PLL fractional spur、24kHz邻道相位噪声、HSE温漂和多Tag同时发送性能必须实测。

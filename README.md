# Hybrid Video — Research Draft

混合数字–模拟背散射视频传输系统的研究构想与当前进度（MetaIoT Research Draft）。

## 推荐的10MHz多节点移频时钟实现

当前优先验证更简洁的 **STM32U575内部PLL1 fractional-N + MCO /8** 方案：通过每个Tag的FRACN配置直接产生10.000–10.216MHz时钟，取消外置LC VCO和本地FLL。

- [PLL1-MCO完整中文设计文档](hardware/pll1_mco_10mhz/Design_PLL1_MCO_ZH.md)
- [标准电路符号SVG图](hardware/pll1_mco_10mhz/schematics/01_pll1_mco_shift.svg)
- [PLL1/MCO配置代码骨架](hardware/pll1_mco_10mhz/pll1_mco_config.c)
- [FRACN频率配置表](hardware/pll1_mco_10mhz/tag_frequency_table.csv)

> 该方案仍需验证HSE参考、PLL fractional spurs、24kHz邻道相噪以及80–81.728MHz CPU时钟下的图像处理负载。NUCLEO-U575ZI-Q的HSE X3可能未装，见设计文档的板级说明。

## 研究文档

**阅读材料：**
- [研究文档（Markdown，可在 GitHub 直接阅读）](docs/Research_Draft.md)
- [研究文档（独立 HTML，包含图像和公式）](docs/Research_Draft.html)

本文材料包含：研究背景与动机、混合 Backscatter 系统架构、数字/模拟频谱预算、10 节点带宽节省计算、KFM-Restore 已有实验、风险与验证路线。

> 重要：8 kHz 模拟预滤波、20 kHz/节点的 RF OBW，以及约 72.8%–76.8% 的频谱节省属于待验证的设计预算，不是已经完成的真实 RF 实验结果。

图像素材位于 [docs/figures/](docs/figures/)。

## 历史备选方案

此前的离散LC VCO + Tag本地FLL方案保留在 [hardware/local_fll_10mhz_revC/](hardware/local_fll_10mhz_revC/)，作为备选研究设计，不再是首选简化方案。

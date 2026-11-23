# Hybrid Video — Research Draft

混合数字–模拟背散射视频传输系统的研究构想与当前进度（MetaIoT Research Draft）。

**当前进度（2026-11-23）：** 单节点视频采集 → 反向散射 → 接收/播放端到端链路已打通；多节点数字 Key TDMA、模拟视频 FDMA、RF 带宽与功耗指标尚待系统级量化验证。

**阅读材料：**
- [研究文档（Markdown，可在 GitHub 直接阅读）](docs/Research_Draft.md)
- [独立 HTML（包含图像和公式）](docs/Research_Draft.html)
- [频率合成、移频与 Tag 本地闭环反馈完整设计](docs/frequency_synthesis_local_feedback_design.md)
- [Path-A 10 MHz→20 MHz 相干倍频 SVG](docs/figures/frequency_path_a.svg)
- [Path-B Clapp VCO SVG](docs/figures/frequency_path_b_clapp.svg)
- [Tag 本地频率检测闭环 SVG](docs/figures/local_feedback.svg)
- [STM32U575/HM0360 资源核对 SVG](docs/figures/pinout_check.svg)
- [单节点混合数字–模拟反向散射 SVG](docs/figures/fig01_hybrid_video_node.svg)
- [多节点数字 TDMA / 模拟 FDMA 共享接收机 SVG](docs/figures/fig02_multinode_tdm_fdma.svg)

本文材料包含：研究背景与动机、混合 Backscatter 系统架构、数字/模拟频谱预算、10 节点带宽节省计算、KFM-Restore 已有实验、风险与验证路线。

> 重要：8 kHz 模拟预滤波、20 kHz/节点的 RF OBW，以及约 72.8%–76.8% 的频谱节省属于待验证的设计预算，不是已经完成的真实 RF 实验结果。

图像素材位于 [docs/figures/](docs/figures/)。
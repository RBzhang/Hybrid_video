# NUCLEO-U575ZI-Q：10.xxx MHz MCO 移频时钟最小工程（Keil5）

**目标：** 仅让 STM32U575 的 **PA8 / MCO1** 持续输出 10.000～10.216 MHz 的 3.3 V CMOS 方波。无需连接摄像头、DAC、ADG902、RF 板或上位机串口。使用板载 **ST-LINK 的 USB 接口**供电/烧录；不需要连接用户 USB Type-C。

> 状态：已提交独立应用源代码及 CubeMX/Keil5 集成说明；**未在实体 NUCLEO-U575ZI-Q 上烧录、编译或测量验证**。本目录不是已生成的完整 MDK-ARM 工程（HAL/CMSIS/启动文件需由 CubeMX 生成），也不是经测试的 RF 信号源。

## 0. 最重要的用户设置：TAG_ID

仅需编辑 [Core/Inc/tag_clock_config.h](Core/Inc/tag_clock_config.h)：

```c
#define TAG_ID 0U // 0~9; 修改这个值后重新编译 + 烧录
```

| TAG_ID | 理论目标 MCO |
| ---: | ---: |
| 0 | 10.000 MHz |
| 1 | 10.024 MHz |
| 2 | 10.048 MHz |
| 3 | 10.072 MHz |
| 4 | 10.096 MHz |
| 5 | 10.120 MHz |
| 6 | 10.144 MHz |
| 7 | 10.168 MHz |
| 8 | 10.192 MHz |
| 9 | 10.216 MHz |

13 位 FRACN 每步约 61.035 Hz，因此不是每个目标都能精确命中。程序按表设置 [0,393,786,1180,1573,1966,2359,2753,3146,3539]。**这些仅为理想参考频率下的理论值**。

## 1. 时钟路线与最初板卡条件

```text
                             +----> SYSCLK = HSI16 = 16 MHz (CPU)
HSI16 (default, no solder) --+
                             +----> PLL1: M=2, N=20+FRACN/8192, R=2
                                       |
                                       +--> PLL1_R = 80.000..81.728 MHz
                                               |
                                               +--> MCO1 /8 -> PA8 (AF0)
                                                     10.000..10.216 MHz
```

- **默认 TAG_CLOCK_REF_HSI16**：无焊桥改造即可验证输出。HSI 是内部 RC；实际中心频率随芯片偏差/温度而变，**不可将其视为精确 24 kHz 多节点 FDMA 参考**。
- **TAG_CLOCK_REF_STLINK8**：在 [tag_clock_config.h](Core/Inc/tag_clock_config.h) 改 `TAG_CLOCK_REF_SOURCE`；要求根据 NUCLEO [UM2861 §7.7](https://www.st.com/resource/en/user_manual/um2861-stm32u575ziq-nucleo-64-board-mb1549-stmicroelectronics.pdf) 确认 ST-LINK MCO 实测为 8 MHz，并设置 SB47 OFF、SB49 ON、SB48 ON、SB4/SB5 OFF（还应确认 R5 实装/通路）。此时 HSE bypass，PLL1 M=1。**不要在未修改焊桥时选择该模式**。
- **TAG_CLOCK_REF_XTAL16**：自行确认并补装 X3/C59/C60 的 16 MHz 晶体方案，按 UM2861 配置 SB47/SB49/SB48 OFF、SB4/SB5 ON。使用 HSE 晶体模式，PLL1 M=2。
- 三种模式都以 PLL 输入 **8 MHz** 为前提，N=20、R=2，MCO /8，因此 **TAG_ID/FRACN 表完全相同**。
- PLL1 **不作为 SYSCLK**，只为 MCO 提供时钟；CPU 恒用 HSI16。默认例程只在上电时选频，**不支持发送过程中动态切换 TAG_ID**。

## 2. 准备软件

Windows 系统：Keil MDK5/µVision5（已安装）、**STM32CubeMX**（需另外安装）、STM32CubeU5 MCU 固件包、Keil Pack Installer 的 **STM32U5xx DFP**。通过 USB 连接 NUCLEO 的 **ST-LINK**（SWD 调试器）接口，安装 ST-LINK 驱动。此工程不依赖外部晶振进行第一次输出测试。

**不要将这里的 `main.c` 与 CubeMX 原先生成的 `main.c` 同时加入构建**。目标设备为 **STM32U575ZIT6Q（NUCLEO-U575ZI-Q）**，使用非 TrustZone 工程；请检查 MCU Option Bytes/TrustZone 状态与工程一致。

## 3. 用 STM32CubeMX 生成完整 Keil5 外壳

1. File → New Project → **MCU Selector**，搜索 `STM32U575ZIT6Q`；不要为了本实验勾选摄像头、DAC、USB Device 或外设中断。
2. Pinout & Configuration → System Core → RCC：保持 **HSI16 enabled**；SYSCLK 选择 **HSI16**，保持 PLL1 不作为 SYSCLK。PA8 不需在 CubeMX 里预配置，源代码负责配置为 AF0/MCO1。
3. Clock Configuration：**SYSCLK/HCLK/PCLK1/PCLK2/PCLK3 = 16 MHz**，APB/AHB 分频均为 /1。不要让 CubeMX 自动把 SYSCLK 改为 PLL1；**不要选择 STOP 低功耗模式**。
4. Project Manager → Project：工程名 `NUCLEO_U575_MCO_TagClock`，Toolchain / IDE 选 **MDK-ARM V5**；Code Generator 选复制必要 HAL 库文件（推荐）。
5. 点击 GENERATE CODE。确认有 `Core/Src/main.c`、`Core/Inc/main.h`、`Drivers/STM32U5xx_HAL_Driver`、启动文件和 `MDK-ARM/*.uvprojx`。
6. **备份**生成的 `Core/Src/main.c`，然后将本目录的 `Core/Src/main.c` **覆盖**生成的版本；把本目录 `Core/Src/tag_clock.c` 和两份 `Core/Inc/tag_clock*.h` 复制到对应目录。
7. 用 µVision 打开生成的 `.uvprojx`。通过 **Project → Manage → Project Items** 将 `Core/Src/tag_clock.c` 加入 Target（**仅把文件复制到目录并不意味着 Keil 自动编译它**）。确认 `Core/Inc` 在 Include Paths，`STM32U575xx` 宏正确；HAL 的 RCC、GPIO、PWR、CORTEX 等库由 CubeMX 生成。
8. 编辑 `tag_clock_config.h` 中的 **TAG_ID**（初次用 0），按 **F7** 构建；Target → Options for Target → Debug 选择 ST-Link Debugger，Utilities 也设置 ST-Link；使用 **F8 Load** 下载，Reset & Run。

如果随后再次通过 CubeMX 生成代码，可能会覆盖本目录的 `main.c`；建议先备份或使用版本控制，在重新生成后重新复制。

## 4. 实际如何测量

- **输出测量点：PA8 / MCO1 = Zio D33（CN10 的 D33 所在行，31 脚），或 CN12 的 23 脚**。后者 Morpho 排针可能未焊接；建议优先使用可接触的 Zio D33 焊盘/插座，参考板上丝印。与示波器 **GND 共地**。
- 在 PA8 直接量测的是约 **0–3.3 V 的数字方波**，不是 2.4 GHz 射频信号，也不是已经降到 1.8 V 的 ADG902 控制信号。
- 用 10× 低电容探头（尽量短的地弹簧）、带宽足够的示波器或频率计。首选 TAG_ID=0，理论约 10 MHz；再切换 TAG_ID=1、5、9 并分别重新烧录，检查频率变化。
- 理想参考时 Tag0: 10.000000 MHz；Tag1: 10.023986816 MHz；Tag5: 10.119995117 MHz；Tag9: 10.216003418 MHz。用 HSI16 时实际读数可能明显偏离这些值，但不同配置应表现出可辨认的相对变化。
- 如果使用 HSE 8/16 MHz，再检查参考输入频率及 PLL 输出误差；对相邻节点 24 kHz 的精度承诺只能基于实测外部参考。

建议记录：TAG_ID、时钟源模式、g_tag_clock_status、g_tag_clock_fracn、g_tag_clock_nominal_hz、PA8 实测频率、占空比、波形电平。

## 5. 故障排查

| 现象 | 优先检查 |
| --- | --- |
| PA8 没有 10 MHz | 是否运行到 `TagClock_Start` 返回；Keil Watch 查看 `g_tag_clock_status`；PA8 是否接对；GPIOA/MCO 是否初始化 |
| `g_tag_clock_status=2` | SYSCLK 不是 HSI，CubeMX 可能将 CPU 改为 PLL1 |
| `g_tag_clock_status=3` | HSE8/HSE16 模式时，焊桥、X3、时钟源/频率不匹配；HSI 模式检查 HSI ready |
| `g_tag_clock_status=4/5` | PLL 关闭或锁定超时；检查 PLL 输入、VOS 和时钟配置 |
| 编译 `undefined TagClock_Start` | 在 MDK-ARM 工程里显式添加 `tag_clock.c` |
| 编译 HAL/LL 头文件不存在 | 检查 CubeMX 已复制 STM32CubeU5 HAL/CMSIS，Pack 选择正确 |
| Tag0 不是精确 10.000000 MHz | 默认 HSI16 非高精度参考，确认实际频差；需要精确频率用 HSE |

`g_tag_id` 与 `g_tag_clock_status` 均为 `volatile`，可以在 Keil Watch 查看。**在程序运行过程中改 Watch 中的 g_tag_id 不会自动切换频率**；要改输出频点，请更改 `TAG_ID` 后重新编译烧录。

## 6. 原理与测试脚本

- `tools/print_tag_frequencies.py`：在 PC 上运行 `python tools/print_tag_frequencies.py`，打印 FRACN 理论频率和量化误差；不连接开发板。
- 设计背景：[上一层完整 PLL1/MCO 文档](../Design_PLL1_MCO_ZH.md)；实板交付以本目录的**纯 MCO 输出工程**为准。
- ST 官方资料：[NUCLEO UM2861](https://www.st.com/resource/en/user_manual/um2861-stm32u575ziq-nucleo-64-board-mb1549-stmicroelectronics.pdf)、[STM32U5 RM0456](https://www.st.com/resource/en/reference_manual/rm0456-stm32u5-series-armbased-32bit-mcus-stmicroelectronics.pdf)。

**验收条件：** 使用 Keil 实际烧录并在 PA8 上测得期望量级的连续方波，切换 TAG_ID 后输出随之改变；未实测前不得声称本工程已运行通过。

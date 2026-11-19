# NUCLEO-U575ZI-Q / PLL1-MCO 10 MHz：纯 Keil MDK5 工程

**无须安装 STM32CubeMX，无须生成 HAL/CubeMX 工程。**

本目录包含可在 Keil µVision5 打开的 [MDK-ARM/NUCLEO_U575_TagClock.uvprojx](MDK-ARM/NUCLEO_U575_TagClock.uvprojx)、ST 官方 Cortex-M33 启动文件、最小 SystemInit 和寄存器级 PLL1/GPIO 配置。

> 本工程只在 PA8 / MCO1 输出约 10.000–10.216 MHz 的 **0–3.3 V 数字方波**，不是 2.4 GHz RF。没有摄像头、DAC、ADG902、UART。已做寄存器配置及数值检查；**未用实物 Keil 编译/下载或测示波器**。

## 1. TAG_ID 只需修改一个位置

打开 `Core/Inc/tag_clock_config.h` 修改：

```c
#define TAG_ID 0U   /* 改为 0U..9U */
```

改后在 Keil 按 F7 Build，Load 下载、Reset/Run。每次启动按照 TAG_ID 选频，不支持运行中改变 TAG_ID。

| TAG_ID | FRACN | 理论中心频率 |
|---:|---:|---:|
| 0 | 0 | 10.000000000 MHz |
| 1 | 393 | 10.023986816 MHz |
| 2 | 786 | 10.047973633 MHz |
| 3 | 1180 | 10.072021484 MHz |
| 4 | 1573 | 10.096008301 MHz |
| 5 | 1966 | 10.119995117 MHz |
| 6 | 2359 | 10.143981934 MHz |
| 7 | 2753 | 10.168029785 MHz |
| 8 | 3146 | 10.192016602 MHz |
| 9 | 3539 | 10.216003418 MHz |

**注意：** 所有理论值都假定内部 HSI16 恰好为 16.000000 MHz。HSI16 为 RC，不可作为严格的 24 kHz FDMA 频率标准；可先用于硬件出波/可调性测试，待需要精密频率时换 HSE 设计。

## 2. 软件要求（完全 Keil 内完成）

- **Keil MDK5 / µVision5**，建议 Arm Compiler **6**。
- 在 **Pack Installer** 安装 **Keil::STM32U5xx_DFP**（例如 3.2.0），选择 MCU **STM32U575ZITx**；同时确保有常规 Arm Compiler 6 工具链。
- 不要打开或安装 CubeMX。本工程没有引用 `stm32u575xx.h`、HAL、LL，也没有引用 Device Pack 的 Startup：官方启动汇编文件已经随仓库提供。因此 **DFP 3.x 已移除 Device:Startup 不影响本工程**。
- 用 NUCLEO 板载 **ST-LINK USB 接口**供电/烧录，另一 USB Type-C 对此次输出测试不是必需。
- **MCU Option Bytes 的 TrustZone 必须处于关闭（TZEN=0）**；工程使用非安全地址与普通单镜像启动。若 TZEN=1，不应强行刷此非 TrustZone 项目，应先核对设备原有安全配置。

## 3. 操作步骤

1. 克隆或下载当前 GitHub 分支，打开本目录下 `MDK-ARM/NUCLEO_U575_TagClock.uvprojx`。
2. 若提示 Pack 未安装，在 Keil **Pack Installer** 安装 STM32U5xx_DFP，然后返回 µVision 重新选择 `STM32U575ZITx` 设备。无需添加 `Device:STM32Cube Framework`，无需运行 CubeMX。
3. 确认 **Options for Target → Target → ARM Compiler 6** 选择本机已安装版本；IROM1 起点 `0x08000000`、IRAM1 起点 `0x20000000`。
4. 在 `Core/Inc/tag_clock_config.h` 改 `TAG_ID`，初始先用 `0U`。
5. **F7 Build**：目标为生成 AXF 和 HEX；如出现编译器版本选择弹窗，切换到已安装的 AC6 版本。
6. **Options for Target → Debug** 选 `ST-Link Debugger`；**Utilities** 也配置 ST-LINK Flash Download；确认添加对应 STM32U5 2MB Flash Algorithm（Pack 提供）。
7. 使用 Load（通常 F8），Reset 后 Run。不要进入 STOP/STANDBY，否则 PLL 停止，MCO 断续。
8. 以低电容 10× 探头测量 **PA8 / MCO** 与 GND，首测约 10 MHz 方波；逐次改 TAG_ID=1、5、9 重编译/烧录，观察频点改变。

如果工程打开后提示丢失预配置 Flash Algorithm，请进入 **Options for Target → Utilities → Settings → Flash Download**，清理失效算法并选择该芯片相应的 2MB 内部 Flash 算法。不要选外部 Flash。

## 4. 调试与测量

Keil Watch 中可读取：

| 变量 | 作用 |
|---|---|
| `g_selected_tag_id` | 上电时采用的 TAG_ID |
| `g_tag_clock_status` | 0 为配置成功；1=ID错误；2=原 SYSCLK 异常；3=HSI超时；4=PLL停机超时；5=PLL锁定超时 |
| `g_tag_clock_id` | 已配置完成的 Tag |
| `g_tag_clock_fracn` | 已采用的 FRACN |
| `g_tag_clock_nominal_hz` | 基于理想 16 MHz 参考的整数 Hz 值 |

硬件连接：**PA8/MCO 对应 Zio D33 或 Morpho CN12-23**（使用板卡丝印再确认插座/焊盘），示波器地与开发板 GND 共地。PA8 原生输出 3.3 V，**不能直接接入 1.8 V 供电的 ADG902 CTRL**。

## 5. 设计原理与工程文件

- `Core/Inc/tag_clock_config.h` — 唯一用户配置：`TAG_ID`
- `Core/Src/main.c` — 最小主程序
- `Core/Src/tag_clock.c` — RCC/PLL1/GPIO 寄存器操作
- `Core/Inc/tag_clock_hw.h` — 仅工程所需 RCC/GPIO 地址、寄存器定义
- `Core/Src/system_stm32u5xx.c` — 实现 `SystemInit` 以及时钟变量
- `MDK-ARM/startup_stm32u575xx.s` — ST 官方 MDK-ARM 启动文件，保留许可证 `MDK-ARM/LICENSE_ST_CMSIS.md`
- `MDK-ARM/NUCLEO_U575_TagClock.uvprojx` — 可在 Keil5 中打开的工程文件
- `tools/print_tag_frequencies.py` — PC 端理论频率校核程序，Keil 编译不使用该文件

时钟链路：

```text
复位默认 MSIS -> CPU SYSCLK (保持不变)
HSI16 (16 MHz) -> PLL1 M=2 -> N=(20 + FRACN/8192) -> R=2
              -> PLL1_R=80...81.728 MHz -> MCO /8 -> PA8 ~10 MHz
```

由于 CPU 不使用 PLL1，本工程不将 SYSCLK 升高到 80 MHz。所有模式只需修改 TAG_ID，编译下载后立即固定输出。

## 6. 硬件验收

- 在 PA8 测量到稳定 3.3 V CMOS 方波；能够通过 10× 示波器探头清晰观察 10 MHz 周期；
- TAG_ID=0、1、5、9 的测量频率能区别开，改变趋势匹配理论频点；
- 允许 HSI16 不准造成绝对频差，**不得以 10.000000 MHz 的精确值做首版验收**；
- 必须观察 `g_tag_clock_status=0`，并留下编译日志及实测记录；在此之前仅能称为已提交、待实测工程。

官方依据：[RM0456](https://www.st.com/resource/en/reference_manual/rm0456-stm32u575585-armbased-32bit-mcus-stmicroelectronics.pdf)、[ST CMSIS Device U5](https://github.com/STMicroelectronics/cmsis-device-u5)、[Keil STM32U5 DFP](https://www.keil.arm.com/packs/stm32u5xx_dfp-keil/overview/)。

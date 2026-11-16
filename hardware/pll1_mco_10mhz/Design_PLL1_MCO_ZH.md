# STM32U575 内部 PLL1 分数频率合成 10 MHz 移频时钟设计说明

> **版本：** PLL1-MCO Rev.A（详细设计草案）  
> **目标平台：** STM32U575 / NUCLEO-U575ZI-Q  
> **目标：** 每个 Tag 直接输出 10.000 MHz～10.216 MHz 时钟，按 24 kHz 递增，驱动 ADG902 反射开关。  
> **推荐架构：** 外部 HSE 晶体 → STM32U575 PLL1 fractional-N → PLL1_R → MCO /8 → 电平适配 → ADG902 CTRL。  
> **重要：** 本方案取消外置 LC VCO、VCO 调谐 DAC、÷16 分频器和 Tag 本地 FLL。PLL 内部仍有锁相反馈，但不再需要外部测频反馈。频率稳定性取决于 HSE 和 PLL 的实际相位噪声/杂散。

## 1. 设计结论与范围

![PLL1-MCO移频时钟原理图](schematics/01_pll1_mco_shift.svg)

STM32U575 内部 PLL1 支持 13 bit FRACN 分数配置。通过改变 FRACN，可以改变 PLL1_R 输出频率；再使用 MCO 的 /8 预分频，直接获得 10.xxx MHz 的方波时钟。该时钟经电平适配后驱动 ADG902 CTRL，外部 CW 经 Tag 的周期性反射阻抗切换后产生对应频移分量。

本方案的关键优点：

1. 不需要外置 10 MHz LC 振荡器、变容调谐网络和专用 PLL 芯片。
2. 不需要 FPGA 80 MHz NCO，也不需要 10 MHz 与 24 kHz 方波 AND。
3. 不需要 Tag 内部额外计数器、÷16 触发器和 DAC 频率反馈闭环。
4. 每个 Tag 只需根据自身 ID 设置一次 FRACN；PLL 锁定后持续输出。
5. STM32 的 PA5 图像 DAC 和图像变容二极管 D2 完全保留，和 PLL 频率设置独立。

本文件仅设计**移频时钟合成及其到 RF Switch 的接口**，不包含 D2 的 2.4 GHz 匹配、变容偏置网络和天线匹配。这些仍需独立的 RF 设计和实测。

## 2. PLL1 与 MCO 的频率关系

### 2.1 PLL1 基本关系

令 HSE 输入频率为 \(f_{\rm HSE}\)，PLL1 分频/倍频参数为 M、N、R，分数系数为 FRACN，则

$$
f_{\rm VCO}=\frac{f_{\rm HSE}}{M}
\left(N+\frac{\rm FRACN}{8192}\right)
$$

PLL1_R 输出为

$$
f_{\rm PLL1\_R}=\frac{f_{\rm VCO}}{R}.
$$

本方案选择 MCO 时钟源为 PLL1 主时钟输出（PLL1_R / PLLCLK），MCO 分频为 8：

$$
f_{\rm MCO}=\frac{f_{\rm HSE}}{M R \cdot 8}
\left(N+\frac{\rm FRACN}{8192}\right).
$$

### 2.2 推荐配置

使用 16 MHz 外部 HSE 参考：

| 参数 | 配置 |
|---|---:|
| HSE | 16 MHz |
| PLL1M | 1 |
| PLL1N | 10 |
| PLL1R | 2 |
| PLL1FRACN | 0～1769（本系统10个节点） |
| PLL1 输入频率范围 | 8～16 MHz |
| PLL1 EPOD/MBOOST 预分频 | DIV1（16 MHz输入） |
| PLL1_R 输出使能 | 开启 |
| PLL1_P、PLL1_Q 输出 | 关闭 |
| MCO 源 | PLLCLK（PLL1_R） |
| MCO 分频 | /8 |
| MCO 引脚 | PA8 / AF0 |
| VOS | Range 2（HCLK不超过110 MHz） |

代入得到：

$$
f_{\rm VCO}=160\ {\rm MHz}
+1953.125\cdot{\rm FRACN}\ {\rm Hz}
$$

$$
f_{\rm PLL1\_R}=80\ {\rm MHz}
+976.5625\cdot{\rm FRACN}\ {\rm Hz}
$$

$$
\boxed{
f_{\rm MCO}=10\ {\rm MHz}
+122.0703125\cdot{\rm FRACN}\ {\rm Hz}
}
$$

因此 MCO 频率设置分辨率为

$$
\Delta f_{\rm MCO}=\frac{16\ {\rm MHz}}{1\cdot2\cdot8\cdot8192}
=122.0703125\ {\rm Hz}.
$$

FRACN 为 13 bit，合法范围 0～8191。本方案的最高节点只需 FRACN=1769，远未达到上限。

### 2.3 10个Tag的FRACN配置表

目标频率为

$$
f_i^*=10\ {\rm MHz}+i\cdot24\ {\rm kHz},\quad i=0,\ldots,9.
$$

计算规则：

$$
{\rm FRACN}_i=
{\rm round}\left(
\frac{f_i^*-10\ {\rm MHz}}{122.0703125\ {\rm Hz}}
\right).
$$

| Tag ID | FRACN | 实际 MCO 频率 (MHz) | 相对目标误差 (Hz) |
|---:|---:|---:|---:|
| 0 | 0 | 10.000000000 | 0.0 |
| 1 | 197 | 10.024047852 | +47.9 |
| 2 | 393 | 10.047973633 | −26.4 |
| 3 | 590 | 10.072021484 | +21.5 |
| 4 | 786 | 10.095947266 | −52.7 |
| 5 | 983 | 10.119995117 | −4.9 |
| 6 | 1180 | 10.144042969 | +43.0 |
| 7 | 1376 | 10.167968750 | −31.3 |
| 8 | 1573 | 10.192016602 | +16.6 |
| 9 | 1769 | 10.215942383 | −57.6 |

上述频率表假设 HSE 恰好为 16.000000 MHz。实际输出频率会随每块板的 HSE 频率同比例变化；FRACN 量化误差并不等于晶体绝对频率误差。

## 3. PLL、CPU 与 MCO 时钟树的关系

本推荐配置下：

- PLL1 VCO：160.000～163.455 MHz；
- PLL1_R：80.000～81.728 MHz；
- SYSCLK 选择 PLL1_R；
- AHB prescaler = 1；
- HCLK = SYSCLK = 80.000～81.728 MHz；
- MCO = PLL1_R / 8 = 10.000～10.216 MHz。

STM32U575 的系统时钟最高可到 160 MHz；本配置下 HCLK 不超过 81.728 MHz，建议采用 VOS Range 2（该电压范围的 HCLK 上限为 110 MHz）。PLL1 的 VCO 频率和 PLL1_R 输出频率必须分别按数据手册约束检查；本方案 VCO 约160～163.5MHz、R输出约80～81.7MHz。

**重要的性能取舍：** 如果把 PLL1_R 同时作为 SYSCLK，MCU 核心频率将随 Tag 频点从 80MHz 变化到约81.728MHz。这低于 NUCLEO 默认常见的160MHz配置。对 320×240、1 FPS 的 DCMI+DMA、25.6～100 kSa/s DAC 和65-tap FIR，80MHz可能足够，但必须在实际固件中测量 CPU 占用和最坏执行时间，不能仅凭理论假设。

如果系统不能接受约80MHz CPU，则此单PLL共享方案需要重新权衡；单独用 PLL1 生成 MCO、CPU 改用其他时钟源会导致 CPU 时钟更低，增加第二套时钟合成方案则会增加功耗和复杂度。

## 4. NUCLEO-U575ZI-Q 的 HSE 参考时钟

### 4.1 不要假设板载16MHz晶体已经安装

NUCLEO-U575ZI-Q 的 MB1549-C05 原理图把外部 HSE X3（NX2016SA 16MHz）及其负载电容位置标为可选/未装。必须先检查你手中的板卡版本和 X3 实物，不能直接假设默认已有16MHz HSE。

NUCLEO 用户手册列出的板载 HSE 晶体示例参数为16MHz、8pF、20ppm，并说明相应焊桥设置：

- SB47、SB49 OFF；
- SB48 OFF；
- SB4、SB5 ON；
- X3 与 C59/C60 按晶体负载电容要求安装。

C59/C60 不应机械照抄固定值。应根据晶体 CL、PCB寄生和 STM32 HSE 振荡器设计指南 AN2867 计算。

### 4.2 实验室最快验证方式：外部16MHz有源振荡器

若板载 X3 未安装，第一轮验证可使用16MHz CMOS 有源振荡器模块：

- 输出接 PH0/OSC_IN（NUCLEO CN11 pin 29）；
- STM32 配置 HSE bypass；
- SB47、SB49 ON；
- SB48 OFF；
- SB4、SB5 OFF；
- PH1 可按板级设置作为 GPIO（具体检查 SB49/板卡布线）。

外部时钟输出电平需满足 PH0 输入电气要求，建议3.3V CMOS、短走线、就近去耦。

### 4.3 NUCLEO-U575ZI-Q 的临时实验时钟：ST-LINK MCO

NUCLEO-U575ZI-Q 的 ST-LINK 参考时钟必须按**实际板卡版本和 ST-LINK 固件**确认，不能笼统假设为5MHz或8MHz：

- UM2861 Rev.10 对 ST-LINK-V3E 的默认 MCO 描述为8MHz，且可能随 ST-LINK 固件升级变化；默认8MHz由HSI/2产生，不应未经校准就当作高精度参考。
- ST 官方社区的时钟配置说明指出，ST-LINK-V3 MCO可通过固件升级工具配置；若选择由ST-LINK外部HSE产生的5MHz（例如25MHz/5），比HSI/2的8MHz更适合作为实验参考。
- 另有NUCLEO-U575案例使用ST-LINK/V2 MCO 5MHz。不同板卡版本不可混用其焊桥/频率假设。

**实验步骤：** 先用示波器/频率计测量PH0实际输入频率，再将CubeMX的HSE_VALUE、HSE bypass模式和PLL参数设置为相同数值。ST-LINK MCO仅用于单板bring-up，不作为最终多Tag产品的独立频率基准。

如果PH0实测为8MHz，保持R=2、MCO /8，将N改为20，PLL输入范围设为8～16MHz：

$$
f_{\rm MCO}=10\,{\rm MHz}+61.03515625\cdot\rm FRACN\ {\rm Hz}.
$$

FRACN可使用：0、393、786、1180、1573、1966、2359、2753、3146、3539。

如果PH0实测为5MHz，保持R=2、MCO /8，将N改为32，PLL输入范围设为4～8MHz：

$$
f_{\rm MCO}=10\,{\rm MHz}+38.14697265625\cdot\rm FRACN\ {\rm Hz}.
$$

FRACN可使用：0、629、1258、1887、2517、3146、3775、4404、5033、5662。

两种实验输入都使PLL1 VCO保持在160～163.455MHz，PLL1_R保持在80～81.728MHz。**最终Tag仍建议使用独立16MHz HSE晶体或规格明确的外部参考源。**

### 4.4 HSE精度与多节点频率误差

若最终每个Tag使用总频率误差为±20ppm的16MHz晶体，则在10.0～10.216MHz处，单节点绝对频率误差约为±200～204Hz。相邻两块板最坏相反误差约400～409Hz，因此名义24kHz中心间隔可能缩小至约23.59kHz。

如果单节点实际RF占用带宽确实为20kHz，剩余名义保护带约3.6kHz；这还没有计入PLL fractional spurs、近端相位噪声、开关谐波和温度漂移。若希望更大余量，应选总频率稳定度更好的晶体（例如10ppm或更好），并在整机温度范围内验证。

## 5. PLL1 初始化和寄存器配置顺序

### 5.1 初始化顺序

1. 先启动并等待 HSE ready；
2. 如果当前 SYSCLK 正由 PLL1 提供，先将 SYSCLK 临时切到 HSE/MSI，再关闭 PLL1；
3. 设置 PWR VOS Range 2，并等待电压缩放就绪；
4. 设置 PLL1 source = HSE；
5. 最终16MHz HSE配置PLL1M=1、PLL1N=10、PLL1R=2；5MHz或8MHz实验参考按4.3节更改N；
6. 设置 PLL1RGE：16MHz HSE使用8～16MHz；5MHz实验参考使用4～8MHz；8MHz实验参考使用8～16MHz；
7. 设置 PLL1 MBOOST/EPOD prescaler = DIV1；
8. 关闭不需要的 PLL1_P 和 PLL1_Q 输出，仅使能 PLL1_R；
9. 设置 FRACN；Tag0 可使用整数模式，Tag1～9启用 fractional mode；
10. 启用 PLL1，等待 PLL1RDY；
11. 设置 AHB/APB prescaler；
12. 将 SYSCLK 切换到 PLL1_R；
13. 配置 MCO source = PLLCLK（PLL1_R），MCO prescaler = /8；
14. 配置 PA8 为 AF0 MCO、push-pull、very high speed、no pull；
15. 等待 MCO 波形稳定后再开启 CW 激励/ADG902有效反射。

PLL1 的 fractional mode 在配置时应遵循参考手册对 FRACN/FRACEN 的写入顺序。最简单可靠的方式是**每个 Tag 在启动阶段设置一次 FRACN，等待 PLL 锁定后不再动态改频**。如果需要运行中切换 Tag 频点，应先关闭 RF 激励或使 RF Switch 不发送，再按 HAL/LL 规定的 FRACEN 清零、等待参考时钟周期、写入 FRACN、重新使能的流程操作。

### 5.2 LL 配置伪代码

以下是基于 STM32U5 LL RCC 接口的配置骨架；不同 STM32CubeU5 版本的宏名称和外设头文件可能有差异，编译前需以安装版本的 stm32u5xx_ll_rcc.h 为准。

    // 先确认 HSE 已经 ready。
    // 若 PLL1 当前是 SYSCLK，先切换到 HSE/MSI 再关闭 PLL1。

    LL_RCC_PLL1_Disable();
    while (LL_RCC_PLL1_IsReady()) {}

    LL_RCC_PLL1_ConfigDomain_SYS(
        LL_RCC_PLL1SOURCE_HSE,
        1U,     // M
        10U,    // N
        2U      // R
    );

    LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_8_16);
    LL_RCC_SetPll1EPodPrescaler(LL_RCC_PLL1MBOOST_DIV_1);

    LL_RCC_PLL1_DisableDomain_SAI();   // 关闭 P 输出域
    LL_RCC_PLL1_DisableDomain_48M();   // 关闭 Q 输出域
    LL_RCC_PLL1_EnableDomain_SYS();    // 只开启 R 输出域

    if (fracn == 0U) {
        LL_RCC_PLL1FRACN_Disable();    // 整数模式
    } else {
        LL_RCC_PLL1_SetFRACN(fracn);
        LL_RCC_PLL1FRACN_Enable();
    }

    LL_RCC_PLL1_Enable();
    while (!LL_RCC_PLL1_IsReady()) {}

    // VOS、Flash latency、AHB/APB prescaler需在CubeMX/启动代码中配置。
    // SYSCLK切换至PLL1_R后，更新SystemCoreClock及外设时钟配置。

    LL_RCC_ConfigMCO(LL_RCC_MCO1SOURCE_PLLCLK, LL_RCC_MCO1_DIV_8);
    // PA8配置为AF0 MCO，push-pull，very high speed，无上下拉。

代码骨架不包含 HSE 焊桥/旁路设置、PWR VOS等待、Flash latency、超时/故障处理和 GPIO 初始化。请不要直接复制为量产固件。

## 6. PA8 MCO 与 ADG902 的逻辑电平接口

### 6.1 PA8 输出

STM32U575 数据手册的 AF 表确认 PA8 可配置 AF0=MCO。MCO输出频率为10.000～10.216MHz。建议：

- GPIO mode = Alternate Function；
- AF = AF0；
- output type = push-pull；
- speed = very high；
- no pull-up/no pull-down；
- PA8到缓冲器输入走线尽量短；
- 避免较大的探头电容直接挂在PA8。

### 6.2 为什么3.3V MCO不能直接连接1.8V ADG902

ADG902的VDD工作范围为1.65～2.75V；其CTRL输入电压**不得超过VDD**。若ADG902使用1.8V供电，而STM32U575的VDD/PA8输出为3.3V，则不能直接连接。

推荐增加一颗 Nexperia 74AUP1G17：

| 引脚 | 连接 |
|---:|---|
| 1 | NC，不连接 |
| 2 | A，接PA8/MCO |
| 3 | GND |
| 4 | Y，接ADG902 CTRL |
| 5 | VCC，接1.8V |

该器件输入耐受最高3.6V，VCC可工作在0.8～3.6V；1.8V供电时输出1.8V逻辑。其在1.65～1.95V供电下传播延迟最大约6.3ns（指定负载条件），对10MHz、100ns周期的时钟有足够的初步时序余量，但仍需测量实际边沿和负载。

U1附近放置100nF去耦；U1输出到ADG902 CTRL之间可预留22～33Ω串联阻尼电阻。ADG902 VDD=1.8V并就近放置100nF去耦。

**可选简化：** 若最终Tag将 STM32 的VDD/PA8 I/O域也设置为1.8V，且摄像头及其他所有外设都兼容1.8V逻辑，可评估PA8直接驱动1.8V供电的ADG902，省略74AUP1G17。NUCLEO的JP4可选择MCU VDD=3.3V或1.8V；更改前必须确认摄像头、串口、调试和其他外设电平兼容。对现有默认3.3V NUCLEO测试，建议保留U1。

## 7. PLL1 输出作为系统时钟对图像采集的影响

因为本方案将PLL1_R作为SYSCLK，MCU主频不再是固定值：

\[
f_{\rm SYSCLK}=8f_{\rm MCO}=80.000\ldots81.728\ {\rm MHz}.
\]

因此：

1. HAL tick、SysTick、UART baud rate和基于HCLK/PCLK的定时器必须根据实际 FRACN 重新计算；
2. 图像 DAC 的定时器触发频率不能继续使用固定80MHz/160MHz配置；
3. 若APB prescaler=1，则定时器时钟可按HCLK计算：

\[
f_s=\frac{f_{\rm TIM}}{(PSC+1)(ARR+1)}.
\]

例如100kSa/s，Tag0的fTIM=80MHz，PSC=0、ARR=799；Tag9的fTIM≈81.72754MHz，PSC=0、ARR=816，得到约100.0337kSa/s。25.6kSa/s也可按同样方式逐Tag配置。若需要更精确的平均采样率，可考虑DMA交替更新ARR做分数周期抖动，但要评估采样时刻抖动和模拟图像恢复影响。

建议固件在设置FRACN后，根据

\[
f_{\rm SYSCLK}=\frac{f_{\rm HSE}}{M R}
\left(N+\frac{\rm FRACN}{8192}\right)
\]

计算真实时钟，并更新SystemCoreClock、定时器ARR、UART baud等参数。**确认当前STM32CubeU5 HAL的SystemCoreClockUpdate/HAL_RCC_GetSysClockFreq是否正确计入FRACN；若未计入，需在应用中按上述公式修正。**

80～81.7MHz对HM0360的1FPS DCMI+DMA和65-tap FIR可能足够，但必须通过真实固件profiling确认。若CPU负载不够，不能简单把PLL1_R升到160MHz，因为该配置的10.216MHz上限需要PLL1_R约81.728MHz、MCO /8。

## 8. HSE精度、FRACN误差和FDMA保护带

### 8.1 两种误差来源

- FRACN量化：本方案约±61Hz以内（表中最大目标误差约58Hz）。
- HSE误差：例如16MHz晶体总稳定度±20ppm，在10MHz处约±200Hz，在10.216MHz处约±204Hz。

因此最终中心频率精度主要由HSE决定，而不是FRACN分辨率。

### 8.2 24kHz相邻节点的最坏中心间距

若各Tag HSE总误差为±20ppm，相邻节点各自的误差方向最不利时，中心间距可能由24kHz缩小约0.4kHz，约为23.6kHz。

若实际单节点RF占用带宽不超过20kHz，剩余名义保护带约3.6kHz。这一保护带仍需为PLL分数杂散、相位噪声、温漂、开关时钟边沿和强弱节点干扰留出空间。

### 8.3 不能仅凭频率表宣布FDMA可用

必须实测：
- 每个FRACN频点的频谱；
- 距载波±24kHz、±48kHz处的分数杂散与相噪；
- 10个Tag同时工作时的邻道泄漏；
- HSE温度漂移、老化和不同晶体批次的频率偏差；
- RF Switch驱动波形的占空比、边沿和实际反散射信号OBW。

STM32数据手册给出的PLL抖动测试条件并不等于本方案160MHz VCO、24kHz偏移处的完整相位噪声曲线。不能用“FRACN分辨率122Hz”代替频谱纯度验证。

## 9. 功耗估算

本方案不再有外置LC VCO、Q2振荡缓冲、变容调谐D1、四级分频器、外部计频逻辑和DAC反馈电路。

| 功耗项 | 估算/依据 | 说明 |
|---|---|---|
| PLL1核心 | 数据手册给出PLL功耗参考点：VCO=160MHz、Range1、单输出时SMPS约260µA，LDO约370µA | 本配置VCO160～163.455MHz且HCLK为Range2；不是完全相同测试条件，需实测 |
| HSE 16MHz晶体 | 由STM32 HSE振荡器驱动 | 需计入MCU功耗，依晶体CL和驱动等级 |
| PA8 MCO输出 | 负载主要为U1输入约1.1pF及PCB寄生 | 动态功耗随负载、电压、频率变化 |
| 74AUP1G17 | 1.8V供电，输入电容约1.1pF、输出电容约1.7pF | 低静态电流，10MHz动态功耗需实测 |
| ADG902 CTRL | 输入电容约2.1pF，静态电流小于1µA典型 | 另计1.8V逻辑动态负载 |

粗略估计，MCO GPIO、74AUP1G17和ADG902控制端的新增动态功耗处于约0.1～0.4mW量级（不含HSE、PLL及MCU内核）。PLL1核心功耗是MCU运行时钟的一部分；若PLL1本来已作为系统时钟，不能把整个PLL功耗再次作为“额外频移模块功耗”重复相加。

本配置会将CPU频率从常见的160MHz配置降到80～81.7MHz，MCU核心功耗可能下降，但具体收益取决于固件执行时间、总线活动、外设和电源模式。需要分别测量HSE、PLL、MCO、缓冲器和ADG902控制支路的电流。

## 10. 元器件与电路接口清单

| 参考 | 器件/规格 | 数量 | 用途 |
|---|---|---:|---|
| U1 | STM32U575ZI（已有） | 1 | PLL1、MCO、PA5图像DAC |
| X1 | 16MHz HSE晶体，建议10～20ppm总稳定度 | 1 | 多节点频率参考；NUCLEO可选X3位置 |
| C1/C2 | 晶体负载电容，按CL/AN2867计算 | 2 | HSE晶体负载；不直接假设固定6.8pF |
| U2 | 74AUP1G17GW，TSSOP5 | 1 | 3.3V MCO到1.8V ADG902控制电平 |
| R1 | 22～33Ω | 1 | U2输出阻尼，预留DNP/调值 |
| C3 | 100nF | 1 | U2 1.8V去耦 |
| U3 | ADG902BRMZ（已有RF设计） | 1 | 反射式RF开关 |
| C4 | 100nF | 1 | ADG902 VDD去耦 |
| U4（可选） | 1.8V LDO | 1 | 若板上无可用1.8V外设电源 |
| D2 | 图像变容二极管（原方案） | 1 | 图像模拟调制，保持独立 |

**相比原LC VCO方案，频率产生所需的新增元器件主要是一颗HSE晶体（若板上没有）和一颗逻辑缓冲器。** 现有ADG902不属于新增频率合成器件。

## 11. 固件频点设置

Tag ID 可通过 Flash 参数、编译期宏或板上ID电阻读取。启动时按下表取得FRACN：

| Tag ID | FRACN |
|---:|---:|
| 0 | 0 |
| 1 | 197 |
| 2 | 393 |
| 3 | 590 |
| 4 | 786 |
| 5 | 983 |
| 6 | 1180 |
| 7 | 1376 |
| 8 | 1573 |
| 9 | 1769 |

推荐流程：
1. 读取Tag ID；
2. 关闭RF激励或保持RF开关不进入有效传输；
3. 启用HSE并等待稳定；
4. 配置PLL1 M/N/R、FRACN、P/Q/R输出使能；
5. 等待PLL1RDY（数据手册fractional mode锁定时间最大约65µs），建议再留保守的稳定等待时间；
6. 配置SYSCLK、MCO /8、PA8 AF0；
7. 配置U1和ADG902电源；
8. 按实际SYSCLK重新配置DAC采样定时器、SysTick和UART；
9. 先观察PA8频率与波形，再启动CW和模拟图像传输。

本方案**不需要Tag本地频率计数反馈**。若想在研发阶段验证频率，可用外部频率计/示波器，但它不参与Tag运行闭环。

## 12. 验证计划

| 阶段 | 操作 | 验收项目 |
|---|---|---|
| 1 | 检查/安装16MHz HSE | PH0/PH1晶体或外部时钟启动正确 |
| 2 | 单Tag FRACN=0 | PA8输出10.000MHz |
| 3 | 单Tag FRACN=197、393、…、1769 | 频率覆盖10.000～10.216MHz |
| 4 | PA8接U1/ADG902 | CTRL高低电平符合ADG902限制，开关可靠工作 |
| 5 | 频谱仪测PA8和PLL输出 | 分数杂散、相噪、谐波和频率偏差 |
| 6 | 连接CW及RF负载 | 实际反散射频移位置、镜频、OBW |
| 7 | 连接图像DAC/D2 | 图像信号频谱和重建质量 |
| 8 | 多Tag同时工作 | 24kHz邻道干扰、强弱节点、温漂 |
| 9 | MCU运行负载测试 | 80～81.7MHz下DCMI、FIR、DAC DMA实时性 |
| 10 | 功耗分轨测量 | HSE、PLL、MCO、buffer、RF switch控制的电流 |

### 通过条件

- PA8各频点连续、稳定、无丢脉冲；
- FRACN实际频率与公式一致；
- ADG902 CTRL不超过VDD；
- 24kHz相邻节点在真实图像信号下可分离；
- MCU 80～81.7MHz运行仍能按时完成图像采集、滤波和DAC输出；
- PLL fractional spur和近端相位噪声不会显著破坏模拟FDMA。

## 13. 主要风险与边界

1. **HSE未装：** NUCLEO-U575ZI-Q的C05原理图把X3外部HSE晶体和C59/C60列为可选未装。必须检查实板；不要默认有16MHz晶体。
2. **CPU降频：** 本方案SYSCLK为80～81.728MHz，而非160MHz。要重新测量图像算法时限。
3. **频率精度依赖HSE：** FRACN只有量化分辨率，不会校准每块板的晶体偏差。
4. **Fractional spurs：** PLL分数模式可能出现杂散；必须在±24kHz附近测量。
5. **MCO电平：** 3.3V PA8不能直接连接1.8V供电的ADG902 CTRL；保留U2，或确保整个MCU IO域也为1.8V且所有外设兼容。
6. **MCO不是RF输出：** PA8只是10MHz控制时钟，真正2.4GHz反散射由ADG902与变容负载的射频阻抗变化形成。
7. **谐波：** 方波开关控制会产生奇次谐波边带，除镜频外还应测量±3f、±5f等分量。
8. **RF匹配未完成：** 本文件不声称ADG902后的2.4GHz网络已经定型。

## 14. 厂商资料

- [STM32U575 数据手册 DS13737 Rev.10](https://www.st.com/resource/en/datasheet/stm32u575vg.pdf)：PLL、MCO、PA8 AF0、HCLK/VOS限制、PLL功耗和抖动指标。
- [STM32U5 参考手册 RM0456](https://www.st.com/resource/en/reference_manual/rm0456-stm32u575585-armbased-32bit-mcus-stmicroelectronics.pdf)：PLL1、FRACN、RCC、MCO寄存器和时钟切换时序。
- [STM32U5 LL RCC接口](https://github.com/STMicroelectronics/stm32u5xx_hal_driver)：PLL1配置与MCO接口；以项目实际CubeU5版本为准。
- [NUCLEO-U575ZI-Q用户手册 UM2861](https://www.st.com/resource/en/user_manual/um2861-stm32u575ziq-nucleo-64-board-mb1549-stmicroelectronics.pdf)：HSE X3、STLINK MCO和焊桥配置。
- [MB1549-U575ZIQ-C05官方原理图](https://www.st.com/resource/en/schematic_pack/mb1549-u575ziq-c05-schematic.pdf)：X3 HSE可选未装、PH0/PH1、PA8连接。
- [ST官方社区：NUCLEO板FDCAN时钟配置](https://community.st.com/stm32-mcus-60/stm32-fdcan-running-at-8-mb-s-on-nucleo-boards-141121)：列出NUCLEO-U575使用ST-LINK/V2 MCO 5MHz的案例，并要求核对板上ST-LINK版本。
- [ST官方社区：ST-LINK-V3 MCO精确时钟配置](https://community.st.com/stm32-mcus-60/how-to-use-stlink-v3-mco-output-on-nucleo-boards-as-a-precise-clock-source-for-stm32-140173)：说明V3 MCO可由外部HSE分频配置，不能把HSI/2默认8MHz当作高精度参考。
- [Nexperia 74AUP1G17 Rev.14](https://assets.nexperia.com/documents/data-sheet/74AUP1G17.pdf)：输入3.6V耐受、1.8V输出、传播延迟和功耗。
- [ADI ADG901/ADG902 Rev.D](https://www.analog.com/media/en/technical-documentation/data-sheets/adg901_902.pdf)：ADG902 VDD=1.65～2.75V、CTRL不得超过VDD、CTRL VIH/VIL和输入电容。

---

**最终建议：** 先用现有NUCLEO-U575ZI-Q验证16MHz HSE、PLL1 fractional、PA8 MCO和10个FRACN频点。先不要制作LC VCO/FLL板。若PLL输出的相位噪声、分数杂散和多Tag邻道性能通过，再将该方案作为最终Tag的频率产生架构。

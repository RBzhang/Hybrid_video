/*
 * STM32U575 PLL1 fractional-N -> PLL1_R -> MCO /8.
 *
 * Final production/reference plan:
 *   HSE = 16 MHz crystal, M=2, N=20, R=2.
 *   PLL input = 8 MHz; VCO = 160...163.456 MHz.
 *   PLL1_R = 80...81.728 MHz; MCO /8 = 10...10.216 MHz.
 *
 * Bench-only alternative:
 *   HSE bypass = measured 8 MHz external clock, M=1, N=20, R=2.
 *   This gives the same VCO, PLL1_R, MCO and FRACN table.
 *
 * The Tag sets FRACN once at startup. No external FLL, DAC frequency
 * control, counter feedback, or SDR feedback is used.
 *
 * This is a configuration skeleton, not a complete CubeMX project.
 * Add HSE setup, VOS/Flash latency, timeouts, peripheral clock updates,
 * GPIO clock enable, and error handling before compiling/deployment.
 */
#include "main.h"
#include "system_stm32u5xx.h"
#include "stm32u5xx_ll_rcc.h"
#include "stm32u5xx_ll_gpio.h"

#if (HSE_VALUE == 16000000UL)
#define PLL1_M_VALUE       2U
#define PLL1_N_VALUE       20U
#define PLL1_INPUT_RANGE   LL_RCC_PLLINPUTRANGE_8_16
#elif (HSE_VALUE == 8000000UL)
#define PLL1_M_VALUE       1U
#define PLL1_N_VALUE       20U
#define PLL1_INPUT_RANGE   LL_RCC_PLLINPUTRANGE_8_16
#else
#error "Use 16 MHz HSE crystal for final Tag, or measured 8 MHz HSE-bypass clock for bench only."
#endif

#define PLL1_R_VALUE       2U
#define MCO_DIVISOR        8U

static const uint16_t g_tag_fracn[10] = {
    0U, 393U, 786U, 1180U, 1573U,
    1966U, 2359U, 2753U, 3146U, 3539U
};

static uint32_t PLL1_R_Hz_From_FRACN(uint16_t fracn)
{
    /*
     * f_PLL1_R = HSE/M * (N + FRACN/8192) / R.
     * Integer arithmetic truncates sub-Hz fractional remainder.
     */
    const uint64_t numerator =
        (uint64_t)HSE_VALUE *
        ((uint64_t)PLL1_N_VALUE * 8192ULL + (uint64_t)fracn);

    return (uint32_t)(numerator /
        ((uint64_t)PLL1_M_VALUE * PLL1_R_VALUE * 8192ULL));
}

void PLL1_MCO10MHz_Init(uint8_t tag_id)
{
    uint16_t fracn;

    if (tag_id >= 10U) {
        tag_id = 0U;
    }
    fracn = g_tag_fracn[tag_id];

    /*
     * Prerequisites:
     * 1) HSE configured as crystal mode (16 MHz final) or bypass mode
     *    (measured 8 MHz bench source); wait until HSERDY.
     * 2) Set PWR VOS Range 2 and wait until voltage scaling is ready.
     * 3) Configure Flash wait states for actual VDD and HCLK <= 81.728 MHz.
     * 4) If PLL1 currently supplies SYSCLK, switch SYSCLK to HSE/MSI first.
     */

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSE);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSE) {
        /* Add timeout/watchdog handling. */
    }

    LL_RCC_PLL1_Disable();
    while (LL_RCC_PLL1_IsReady()) {
        /* Add timeout/watchdog handling. */
    }

    LL_RCC_PLL1_ConfigDomain_SYS(
        LL_RCC_PLL1SOURCE_HSE,
        PLL1_M_VALUE,
        PLL1_N_VALUE,
        PLL1_R_VALUE
    );

    LL_RCC_PLL1_SetVCOInputRange(PLL1_INPUT_RANGE);
    LL_RCC_SetPll1EPodPrescaler(LL_RCC_PLL1MBOOST_DIV_1);

    /* Only PLL1_R is needed for SYSCLK and MCO; disable P and Q outputs. */
    LL_RCC_PLL1_DisableDomain_SAI();
    LL_RCC_PLL1_DisableDomain_48M();
    LL_RCC_PLL1_EnableDomain_SYS();

    if (fracn == 0U) {
        LL_RCC_PLL1FRACN_Disable();   /* Tag 0: integer mode */
    } else {
        LL_RCC_PLL1_SetFRACN(fracn);
        LL_RCC_PLL1FRACN_Enable();    /* Tags 1...9: fractional mode */
    }

    LL_RCC_PLL1_Enable();
    while (!LL_RCC_PLL1_IsReady()) {
        /* Add timeout/watchdog handling. */
    }

    /* Recommended shared-clock configuration: HCLK = PLL1_R, AHB /1. */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_1);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1) {
        /* Add timeout/watchdog handling. */
    }

    /* HCLK = 80.000...81.728 MHz in this plan. */
    SystemCoreClock = PLL1_R_Hz_From_FRACN(fracn);

    /* MCO source = main PLL clock (PLL1_R), prescaler = /8. */
    LL_RCC_ConfigMCO(LL_RCC_MCO1SOURCE_PLLCLK, LL_RCC_MCO1_DIV_8);

    /* Enable GPIOA clock before this block. PA8 = AF0 MCO. */
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_8, LL_GPIO_AF_0);
    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_8, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_8, LL_GPIO_SPEED_FREQ_VERY_HIGH);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_8, LL_GPIO_PULL_NO);

    /*
     * PA8 is normally in the 3.3 V MCU I/O domain.
     * Do NOT connect it directly to ADG902 CTRL when ADG902 VDD=1.8 V.
     * Use a 1.8 V powered, 3.6 V tolerant 74AUP1G17 buffer.
     *
     * Recalculate timer/DAC/UART clocks from actual HCLK after FRACN selection.
     * Enable RF/CW only after PLL lock and output settling.
     */
}

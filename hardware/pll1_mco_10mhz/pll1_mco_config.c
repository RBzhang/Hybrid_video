/*
 * STM32U575 PLL1 fractional-N -> PLL1_R -> MCO /8
 * Illustrative LL configuration skeleton; check exact STM32CubeU5 version.
 * HSE=16 MHz, M=1, N=10, R=2.
 * SYSCLK = PLL1_R = 80 ... 81.728 MHz.
 * MCO = PLL1_R / 8 = 10 ... 10.216 MHz.
 */
#include "stm32u5xx.h"
#include "stm32u5xx_ll_rcc.h"
#include "stm32u5xx_ll_bus.h"
#include "stm32u5xx_ll_gpio.h"
#include "stm32u5xx_ll_pwr.h"

static const uint16_t g_tag_fracn[10] = {
    0, 197, 393, 590, 786, 983, 1180, 1376, 1573, 1769
};

static uint32_t PLL1_R_Hz_From_FRACN(uint16_t fracn)
{
    /* HSE_VALUE must be 16000000UL and the actual HSE must match it. */
    const uint64_t numerator =
        (uint64_t)HSE_VALUE * (10ULL * 8192ULL + (uint64_t)fracn);
    return (uint32_t)(numerator / (8192ULL * 2ULL));
}

void PLL1_MCO10MHz_Init(uint8_t tag_id)
{
    uint16_t fracn;
    if (tag_id >= 10U) {
        tag_id = 0U;
    }
    fracn = g_tag_fracn[tag_id];

    /*
     * Board prerequisite:
     * HSE 16 MHz crystal or 16 MHz external clock is present and selected.
     * Wait for HSE ready before entering this function.
     *
     * If PLL1 currently supplies SYSCLK, first switch SYSCLK to a safe
     * source (HSE/MSI), then disable/reconfigure PLL1.
     */

    /* VOS Range 2 is sufficient for HCLK <= 110 MHz. Configure and wait
       for voltage scaling ready using the PWR HAL/LL API in your project. */

    LL_RCC_PLL1_Disable();
    while (LL_RCC_PLL1_IsReady()) {
        /* Add timeout/watchdog handling in production firmware. */
    }

    LL_RCC_PLL1_ConfigDomain_SYS(LL_RCC_PLL1SOURCE_HSE, 1U, 10U, 2U);
    LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_8_16);
    LL_RCC_SetPll1EPodPrescaler(LL_RCC_PLL1MBOOST_DIV_1);

    /* Only PLL1_R is used: disable P/SAI and Q/48-MHz domains. */
    LL_RCC_PLL1_DisableDomain_SAI();
    LL_RCC_PLL1_DisableDomain_48M();
    LL_RCC_PLL1_EnableDomain_SYS();

    if (fracn == 0U) {
        LL_RCC_PLL1FRACN_Disable(); /* Tag 0: integer-N */
    } else {
        LL_RCC_PLL1_SetFRACN(fracn);
        LL_RCC_PLL1FRACN_Enable();
    }

    LL_RCC_PLL1_Enable();
    while (!LL_RCC_PLL1_IsReady()) {
        /* Add timeout/watchdog handling in production firmware. */
    }

    /*
     * Set AHB/APB prescalers to /1 as the simple reference configuration.
     * Set FLASH latency from CubeMX for VOS2 and the actual HCLK.
     */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_1);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1) {
    }

    /* Update application clock bookkeeping including FRACN. */
    SystemCoreClock = PLL1_R_Hz_From_FRACN(fracn);

    /*
     * PA8 = AF0 MCO. Configure GPIOA clock and PA8:
     * AF push-pull, very-high speed, no pull-up/down.
     */
    LL_RCC_ConfigMCO(LL_RCC_MCO1SOURCE_PLLCLK, LL_RCC_MCO1_DIV_8);

    /* GPIO PA8 setup is board/application-specific; configure AF0 here. */
    /* LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE); */
    /* LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_8, LL_GPIO_AF_0); */
    /* LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_8, LL_GPIO_OUTPUT_PUSHPULL); */
    /* LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_8, LL_GPIO_SPEED_FREQ_VERY_HIGH); */
    /* LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_8, LL_GPIO_PULL_NO); */

    /*
     * Wait a conservative post-lock interval, then enable the CW exciter.
     * MCO must feed a 1.8-V-compatible interface before ADG902 CTRL when
     * MCU VDD is 3.3 V. Do not connect 3.3 V directly to ADG902 CTRL.
     */
}

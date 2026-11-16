/*
 * STM32U575 PLL1 fractional-N -> PLL1_R -> MCO /8.
 *
 * Final Tag clock source:
 *   HSE 16 MHz crystal, M=1, N=10, R=2.
 *
 * NUCLEO-U575ZI-Q bench-only alternatives:
 *   ST-LINK MCO 8 MHz: HSE bypass, M=1, N=20, R=2.
 *   ST-LINK MCO 5 MHz: HSE bypass, M=1, N=32, R=2.
 *   Check the exact board revision/firmware and MEASURE PH0 before selecting.
 *
 * SYSCLK = PLL1_R = 80 ... 81.728 MHz.
 * MCO = PLL1_R / 8 = 10 ... 10.216 MHz.
 *
 * This is a configuration skeleton, not a complete CubeMX project.
 * Configure HSE mode, VOS, Flash latency, clock timeouts and PA8 GPIO first.
 */
#include "main.h"                 /* HSE_VALUE / HAL configuration */
#include "system_stm32u5xx.h"    /* SystemCoreClock */
#include "stm32u5xx_ll_rcc.h"
#include "stm32u5xx_ll_gpio.h"

#if (HSE_VALUE == 16000000UL)
#define PLL1_N_VALUE       10U
#define PLL1_INPUT_RANGE   LL_RCC_PLLINPUTRANGE_8_16
static const uint16_t g_tag_fracn[10] = {
    0U, 197U, 393U, 590U, 786U, 983U, 1180U, 1376U, 1573U, 1769U
};
#elif (HSE_VALUE == 8000000UL)
#define PLL1_N_VALUE       20U
#define PLL1_INPUT_RANGE   LL_RCC_PLLINPUTRANGE_8_16
static const uint16_t g_tag_fracn[10] = {
    0U, 393U, 786U, 1180U, 1573U, 1966U, 2359U, 2753U, 3146U, 3539U
};
#elif (HSE_VALUE == 5000000UL)
#define PLL1_N_VALUE       32U
#define PLL1_INPUT_RANGE   LL_RCC_PLLINPUTRANGE_4_8
static const uint16_t g_tag_fracn[10] = {
    0U, 629U, 1258U, 1887U, 2517U, 3146U, 3775U, 4404U, 5033U, 5662U
};
#else
#error "Supported HSE_VALUE: 16 MHz final crystal, 8 MHz ST-LINK-V3E default bench, or 5 MHz HSE-based ST-LINK bench."
#endif

static uint32_t PLL1_R_Hz_From_FRACN(uint16_t fracn)
{
    /* PLL1_R = HSE_VALUE * (N + FRACN/8192) / (M*R), M=1, R=2. */
    const uint64_t numerator =
        (uint64_t)HSE_VALUE *
        ((uint64_t)PLL1_N_VALUE * 8192ULL + (uint64_t)fracn);

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
     * Prerequisites:
     * 1) Configure HSE crystal mode for 16 MHz, or HSE bypass for external
     *    5 MHz ST-LINK MCO. Wait until HSE ready.
     * 2) Configure PWR VOS Range 2 and wait VOS ready.
     * 3) Set Flash latency for HCLK <= 81.728 MHz at actual VDD.
     * 4) If PLL1 currently supplies SYSCLK, first switch SYSCLK to a safe
     *    source (HSE/MSI), then disable/reconfigure PLL1.
     */

    LL_RCC_PLL1_Disable();
    while (LL_RCC_PLL1_IsReady()) {
        /* Add timeout/watchdog handling in production firmware. */
    }

    LL_RCC_PLL1_ConfigDomain_SYS(
        LL_RCC_PLL1SOURCE_HSE,
        1U,              /* M */
        PLL1_N_VALUE,    /* N */
        2U               /* R */
    );

    LL_RCC_PLL1_SetVCOInputRange(PLL1_INPUT_RANGE);
    LL_RCC_SetPll1EPodPrescaler(LL_RCC_PLL1MBOOST_DIV_1);

    /* Only PLL1_R is required. Disable P/SAI and Q/48-MHz output domains. */
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

    /* Reference clock plan: AHB/APB prescalers = /1. */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_1);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1) {
    }

    /* Account for the fractional part in application clock bookkeeping. */
    SystemCoreClock = PLL1_R_Hz_From_FRACN(fracn);

    /* PA8 = AF0 MCO, push-pull, very-high speed, no pull-up/down. */
    LL_RCC_ConfigMCO(LL_RCC_MCO1SOURCE_PLLCLK, LL_RCC_MCO1_DIV_8);

    /* Enable GPIOA clock in the application before this GPIO setup. */
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_8, LL_GPIO_AF_0);
    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_8, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_8, LL_GPIO_SPEED_FREQ_VERY_HIGH);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_8, LL_GPIO_PULL_NO);

    /*
     * Wait a conservative post-lock interval before enabling the CW source.
     * When MCU VDD=3.3 V, PA8 is 3.3 V: do not connect directly to an ADG902
     * powered at 1.8 V. Use a 1.8 V input-tolerant buffer (74AUP1G17).
     */
}

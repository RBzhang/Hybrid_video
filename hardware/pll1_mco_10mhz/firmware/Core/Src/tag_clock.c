/*
 * NUCLEO-U575ZI-Q / STM32U575ZI:
 *   16 MHz HSI (or HSE 16 MHz, or ST-LINK 8 MHz bypass)
 *     -> PLL1 input 8 MHz -> N=20+FRACN/8192 -> R=2
 *     -> PLL1_R ~ 80 MHz -> MCO1 /8 -> PA8 AF0 -> ~10 MHz.
 *
 * SYSCLK stays on HSI16: PLL1_R is ONLY selected for the MCO output.
 * Requires STM32CubeU5 classic HAL and LL headers from CubeMX / MDK.
 */
#include "stm32u5xx_hal.h"
#include "stm32u5xx_ll_rcc.h"
#include "tag_clock.h"
#include "tag_clock_config.h"

#define TAG_PLL_TIMEOUT_MS  100U

/* Rounded to nearest FRACN step (~61.03515625 Hz at MCO). */
static const uint16_t s_fracn_table[10] =
{
    0U, 393U, 786U, 1180U, 1573U,
    1966U, 2359U, 2753U, 3146U, 3539U
};

volatile uint8_t  g_tag_clock_active_id = 0xFFU;
volatile uint16_t g_tag_clock_fracn = 0U;
volatile uint32_t g_tag_clock_nominal_hz = 0U;

static tag_clock_status_t ConfigureReference(uint32_t *pll_source, uint32_t *pll_m)
{
#if (TAG_CLOCK_REF_SOURCE == TAG_CLOCK_REF_HSI16)
    if (LL_RCC_HSI_IsReady() == 0U)
    {
        return TAG_CLOCK_ERR_REFERENCE;
    }
    *pll_source = LL_RCC_PLL1SOURCE_HSI;
    *pll_m = 2U;
#else
    RCC_OscInitTypeDef osc = {0};

    /* HSE is for PLL1 only; SYSCLK remains on HSI16. */
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.PLL.PLLState = RCC_PLL_NONE;

#if (TAG_CLOCK_REF_SOURCE == TAG_CLOCK_REF_STLINK8)
    osc.HSEState = RCC_HSE_BYPASS;
    *pll_m = 1U;
#else
    osc.HSEState = RCC_HSE_ON;
    *pll_m = 2U;
#endif

    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        return TAG_CLOCK_ERR_REFERENCE;
    }
    *pll_source = LL_RCC_PLL1SOURCE_HSE;
#endif
    return TAG_CLOCK_OK;
}

tag_clock_status_t TagClock_Start(uint8_t tag_id)
{
    uint32_t pll_source = 0U;
    uint32_t pll_m = 0U;
    uint16_t fracn;
    uint32_t tick;
    GPIO_InitTypeDef gpio = {0};
    tag_clock_status_t result;

    if (tag_id >= 10U)
    {
        return TAG_CLOCK_ERR_TAG_ID;
    }

    /* Never disable PLL1 if it currently provides SYSCLK.
     * CubeMX must generate a SYSCLK=HSI16 baseline, not SYSCLK=PLL1.
     */
    if (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
    {
        return TAG_CLOCK_ERR_SYSCLK_NOT_HSI;
    }

    result = ConfigureReference(&pll_source, &pll_m);
    if (result != TAG_CLOCK_OK)
    {
        return result;
    }

    /* Do not emit a transient high-frequency signal while reconfiguring. */
    LL_RCC_ConfigMCO(LL_RCC_MCO1SOURCE_NOCLOCK, LL_RCC_MCO1_DIV_1);

    /* PLL1 should be off in the generated baseline, but handle re-entry. */
    LL_RCC_PLL1_Disable();
    tick = HAL_GetTick();
    while (LL_RCC_PLL1_IsReady() != 0U)
    {
        if ((HAL_GetTick() - tick) >= TAG_PLL_TIMEOUT_MS)
        {
            return TAG_CLOCK_ERR_PLL_STOP_TIMEOUT;
        }
    }

    fracn = s_fracn_table[tag_id];
    LL_RCC_PLL1_ConfigDomain_SYS(pll_source, pll_m, 20U, 2U);
    LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_8_16);
    LL_RCC_SetPll1EPodPrescaler(LL_RCC_PLL1MBOOST_DIV_1);

    /* R output only; R remains usable as MCO even when SYSCLK is HSI. */
    LL_RCC_PLL1_DisableDomain_SAI();
    LL_RCC_PLL1_DisableDomain_48M();
    LL_RCC_PLL1_EnableDomain_SYS();

    LL_RCC_PLL1FRACN_Disable();
    LL_RCC_PLL1_SetFRACN((uint32_t)fracn);
    if (fracn != 0U)
    {
        LL_RCC_PLL1FRACN_Enable();
    }

    LL_RCC_PLL1_Enable();
    tick = HAL_GetTick();
    while (LL_RCC_PLL1_IsReady() == 0U)
    {
        if ((HAL_GetTick() - tick) >= TAG_PLL_TIMEOUT_MS)
        {
            return TAG_CLOCK_ERR_PLL_LOCK_TIMEOUT;
        }
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    gpio.Pin = GPIO_PIN_8;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF0_MCO;
    HAL_GPIO_Init(GPIOA, &gpio);

    LL_RCC_ConfigMCO(LL_RCC_MCO1SOURCE_PLLCLK, LL_RCC_MCO1_DIV_8);

    g_tag_clock_active_id = tag_id;
    g_tag_clock_fracn = fracn;
    /* f_MCO = 10 MHz + 500000*FRACN/8192. Rounded in whole hertz. */
    g_tag_clock_nominal_hz =
        10000000UL + (uint32_t)(((uint64_t)500000U * fracn + 4096U) / 8192U);
    return TAG_CLOCK_OK;
}

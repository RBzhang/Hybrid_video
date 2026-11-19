#include "tag_clock.h"
#include "tag_clock_hw.h"

#define TC_TIMEOUT_ITERATIONS  (2000000UL)

/* Nominal f_MCO = 10,000,000 + FRACN*500,000/8192 Hz. */
static const uint16_t tag_fracn[10] =
{
    0U, 393U, 786U, 1180U, 1573U,
    1966U, 2359U, 2753U, 3146U, 3539U
};

volatile uint32_t g_tag_clock_id = 0xFFFFFFFFUL;
volatile uint32_t g_tag_clock_fracn = 0UL;
volatile uint32_t g_tag_clock_nominal_hz = 0UL;

/* Main clock source remains reset-default MSIS. No CPU speed change. */
tag_clock_status_t TagClock_Start(uint8_t id)
{
    uint32_t ticks;
    uint32_t fracn;
    uint32_t cfgr;

    if (id >= 10U)
    {
        return TAG_CLOCK_BAD_TAG_ID;
    }

    /* On reset SWS=00 (MSIS). PLL1 must not supply SYSCLK. */
    if ((TC_RCC_CFGR1 & (3UL << 2)) == (3UL << 2))
    {
        return TAG_CLOCK_BAD_SYSCLK;
    }

    /* Disable MCO before configuring divider / source. */
    TC_RCC_CFGR1 &= ~(0x7FUL << 24);

    /* HSI16 enabled for PLL1 only; CPU stays on MSIS. */
    TC_RCC_CR |= TC_RCC_CR_HSION;
    ticks = TC_TIMEOUT_ITERATIONS;
    while ((TC_RCC_CR & TC_RCC_CR_HSIRDY) == 0UL)
    {
        if (--ticks == 0UL)
        {
            return TAG_CLOCK_HSI_TIMEOUT;
        }
    }

    /* PLL1 must be stopped before changing N, M, R, source. */
    TC_RCC_CR &= ~TC_RCC_CR_PLL1ON;
    ticks = TC_TIMEOUT_ITERATIONS;
    while ((TC_RCC_CR & TC_RCC_CR_PLL1RDY) != 0UL)
    {
        if (--ticks == 0UL)
        {
            return TAG_CLOCK_PLL_STOP_TIMEOUT;
        }
    }

    fracn = (uint32_t)tag_fracn[id];

    /*
     * PLL1CFGR:
     * [1:0] SRC=2 (HSI16)
     * [3:2] RGE=3 (8..16 MHz)
     * [4]   FRACEN=1 if fraction!=0
     * [11:8] M-1=1 (M=2); MBOOST=0 (/1)
     * [18]  REN=1 (PEN=QEN=0)
     * PLL1DIVR: N-1=19, P-1=1, Q-1=1, R-1=1.
     * PLL1FRACR: FRACN stored at bits [15:3].
     */
    cfgr = (2UL << 0) | (3UL << 2) | (1UL << 8) | (1UL << 18);
    TC_RCC_PLL1CFGR = cfgr;
    TC_RCC_PLL1DIVR =
        (19UL << 0) | (1UL << 9) | (1UL << 16) | (1UL << 24);
    TC_RCC_PLL1FRACR = (fracn << 3);
    if (fracn != 0UL)
    {
        TC_RCC_PLL1CFGR = cfgr | (1UL << 4);
    }

    TC_RCC_CR |= TC_RCC_CR_PLL1ON;
    ticks = TC_TIMEOUT_ITERATIONS;
    while ((TC_RCC_CR & TC_RCC_CR_PLL1RDY) == 0UL)
    {
        if (--ticks == 0UL)
        {
            return TAG_CLOCK_PLL_LOCK_TIMEOUT;
        }
    }

    /* GPIOA clock enable, then PA8 AF0 MCO1 push-pull, very-high speed. */
    TC_RCC_AHB2ENR1 |= (1UL << 0);
    (void)TC_RCC_AHB2ENR1;
    TC_GPIOA_MODER = (TC_GPIOA_MODER & ~(3UL << 16)) | (2UL << 16);
    TC_GPIOA_OTYPER &= ~(1UL << 8);
    TC_GPIOA_OSPEEDR |= (3UL << 16);
    TC_GPIOA_PUPDR &= ~(3UL << 16);
    TC_GPIOA_AFRH &= ~(0xFUL << 0); /* PA8 AF0 */

    /* RCC_CFGR1 MCOSEL=5 (PLL1_R), MCOPRE=3 (/8).
     * Configure prescaler and output source together, after PLL is locked.
     */
    TC_RCC_CFGR1 =
        (TC_RCC_CFGR1 & ~(0x7FUL << 24)) |
        (5UL << 24) | (3UL << 28);

    g_tag_clock_id = id;
    g_tag_clock_fracn = fracn;
    g_tag_clock_nominal_hz = 10000000UL +
        (uint32_t)(((uint64_t)500000UL * fracn + 4096UL) / 8192UL);

    return TAG_CLOCK_OK;
}

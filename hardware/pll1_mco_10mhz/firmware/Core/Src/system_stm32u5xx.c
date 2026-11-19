/*
 * Minimal system initialization for Keil CMSIS-compatible ST startup.
 * Reset starts with MSIS as SYSCLK; no TrustZone setup/flash changes needed.
 * The MCO driver independently enables HSI16 and configures PLL1.
 */
#include <stdint.h>

uint32_t SystemCoreClock = 4000000UL; /* nominal reset MSIS SYSCLK */

void SystemInit(void)
{
    /* Intentionally no clock reconfiguration before main(). */
}

void SystemCoreClockUpdate(void)
{
    /* This minimal project never changes SYSCLK from reset default MSIS. */
    SystemCoreClock = 4000000UL;
}

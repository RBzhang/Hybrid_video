#ifndef TAG_CLOCK_HW_H
#define TAG_CLOCK_HW_H
#include <stdint.h>

/*
 * STM32U575 register subset; no CubeMX, HAL, LL or vendor header required.
 * Source: RM0456 RCC/GPIO chapters, ST CMSIS STM32U575 peripheral map.
 * The non-secure peripheral addresses are used (board TZEN must be disabled).
 * Only fields used by this single-purpose MCO project are defined.
 */
#define TC_REG(addr)        (*(volatile uint32_t *)(uintptr_t)(addr))
#define TC_RCC_BASE        (0x46020C00UL)
#define TC_GPIOA_BASE      (0x42020000UL)

#define TC_RCC_CR          TC_REG(TC_RCC_BASE + 0x00UL)
#define TC_RCC_CFGR1       TC_REG(TC_RCC_BASE + 0x1CUL)
#define TC_RCC_PLL1CFGR    TC_REG(TC_RCC_BASE + 0x28UL)
#define TC_RCC_PLL1DIVR    TC_REG(TC_RCC_BASE + 0x34UL)
#define TC_RCC_PLL1FRACR   TC_REG(TC_RCC_BASE + 0x38UL)
#define TC_RCC_AHB2ENR1    TC_REG(TC_RCC_BASE + 0x8CUL)

#define TC_GPIOA_MODER     TC_REG(TC_GPIOA_BASE + 0x00UL)
#define TC_GPIOA_OTYPER    TC_REG(TC_GPIOA_BASE + 0x04UL)
#define TC_GPIOA_OSPEEDR   TC_REG(TC_GPIOA_BASE + 0x08UL)
#define TC_GPIOA_PUPDR     TC_REG(TC_GPIOA_BASE + 0x0CUL)
#define TC_GPIOA_AFRH      TC_REG(TC_GPIOA_BASE + 0x24UL)

#define TC_RCC_CR_HSION      (1UL << 8)
#define TC_RCC_CR_HSIRDY     (1UL << 10)
#define TC_RCC_CR_PLL1ON     (1UL << 24)
#define TC_RCC_CR_PLL1RDY    (1UL << 25)

#endif

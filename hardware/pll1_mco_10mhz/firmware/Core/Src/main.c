/*
 * Minimal STM32CubeMX/MDK-ARM application.
 * Replace generated Core/Src/main.c with this file, and add tag_clock.c.
 * Only function: continuously output the selected 10.xxx MHz MCO on PA8.
 */
#include "main.h"
#include "tag_clock_config.h"
#include "tag_clock.h"

/* Convenient debugger-visible variable. Edit TAG_ID in tag_clock_config.h. */
volatile uint8_t g_tag_id = TAG_ID;
volatile tag_clock_status_t g_tag_clock_status = TAG_CLOCK_ERR_REFERENCE;

void SystemClock_Config(void);
void Error_Handler(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    g_tag_clock_status = TagClock_Start((uint8_t)g_tag_id);

    if (g_tag_clock_status != TAG_CLOCK_OK)
    {
        Error_Handler();
    }

    /* Do not enter STOP/STANDBY: those modes shut down PLL1. */
    while (1)
    {
        __NOP();
    }
}

/* CPU/buses stay at HSI16 = nominal 16 MHz, independent of TAG_ID. */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2) != HAL_OK)
    {
        Error_Handler();
    }

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        Error_Handler();
    }

    clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                    RCC_CLOCKTYPE_PCLK3;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV1;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    clk.APB3CLKDivider = RCC_HCLK_DIV1;

    /* FLASH_LATENCY_1 is a conservative setting at 16 MHz in Range 2. */
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_1) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        /* Inspect g_tag_clock_status in Keil's Watch window. */
    }
}

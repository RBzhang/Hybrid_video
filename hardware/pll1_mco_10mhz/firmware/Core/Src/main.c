/*
 * Keil MDK5 / Arm Compiler 6 / STM32U575ZITx.
 * No CubeMX, HAL or LL libraries. Only PLL1_MCO output on PA8.
 */
#include <stdint.h>
#include "tag_clock_config.h"
#include "tag_clock.h"

/* View in Keil Watch; change TAG_ID in tag_clock_config.h then rebuild. */
volatile uint32_t g_selected_tag_id = TAG_ID;
volatile uint32_t g_tag_clock_status = TAG_CLOCK_BAD_TAG_ID;

int main(void)
{
    g_tag_clock_status = (uint32_t)TagClock_Start((uint8_t)g_selected_tag_id);

    for (;;)
    {
        /* Keep PLL running; never enter STOP/STANDBY low-power modes. */
    }
}

#ifndef TAG_CLOCK_H
#define TAG_CLOCK_H

#include <stdint.h>

typedef enum
{
    TAG_CLOCK_OK = 0,
    TAG_CLOCK_ERR_TAG_ID = 1,
    TAG_CLOCK_ERR_SYSCLK_NOT_HSI = 2,
    TAG_CLOCK_ERR_REFERENCE = 3,
    TAG_CLOCK_ERR_PLL_STOP_TIMEOUT = 4,
    TAG_CLOCK_ERR_PLL_LOCK_TIMEOUT = 5
} tag_clock_status_t;

/* Call once after HAL_Init() and SystemClock_Config().
 * Does not alter SYSCLK: CPU stays on the 16 MHz HSI oscillator.
 */
tag_clock_status_t TagClock_Start(uint8_t tag_id);

/* Debugger watch expressions: nominal values based on ideal reference. */
extern volatile uint8_t  g_tag_clock_active_id;
extern volatile uint16_t g_tag_clock_fracn;
extern volatile uint32_t g_tag_clock_nominal_hz;

#endif /* TAG_CLOCK_H */

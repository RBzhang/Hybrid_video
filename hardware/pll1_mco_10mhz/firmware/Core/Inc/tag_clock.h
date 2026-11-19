#ifndef TAG_CLOCK_H
#define TAG_CLOCK_H
#include <stdint.h>

typedef enum
{
    TAG_CLOCK_OK              = 0,
    TAG_CLOCK_BAD_TAG_ID      = 1,
    TAG_CLOCK_BAD_SYSCLK      = 2,
    TAG_CLOCK_HSI_TIMEOUT     = 3,
    TAG_CLOCK_PLL_STOP_TIMEOUT= 4,
    TAG_CLOCK_PLL_LOCK_TIMEOUT= 5
} tag_clock_status_t;

tag_clock_status_t TagClock_Start(uint8_t id);

/* Watch expressions in Keil Debugger, do not modify while running. */
extern volatile uint32_t g_tag_clock_id;
extern volatile uint32_t g_tag_clock_fracn;
extern volatile uint32_t g_tag_clock_nominal_hz;

#endif

#ifndef TAG_CLOCK_CONFIG_H
#define TAG_CLOCK_CONFIG_H

/* CHANGE ONLY TAG_ID (0..9), then BUILD and DOWNLOAD from Keil. */
#define TAG_ID  0U

#if (TAG_ID > 9U)
#error "TAG_ID must be 0..9"
#endif

/* Nominal f_MCO = 10 MHz + TAG_ID * 24 kHz.
 * Internal HSI16 is suitable for bring-up, NOT a precision FDMA reference.
 * No camera, DAC, RF or external HSE oscillator required.
 */
#endif

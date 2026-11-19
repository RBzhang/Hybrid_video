#ifndef TAG_CLOCK_CONFIG_H
#define TAG_CLOCK_CONFIG_H

/*
 * USER CONFIGURATION — edit TAG_ID only to choose one of ten RF shift clocks.
 * TAG_ID = 0..9 -> nominal MCO = 10.000 MHz + TAG_ID * 24 kHz.
 * Edit, rebuild, and flash. Tag ID is read once at startup.
 */
#define TAG_ID                   0U

/*
 * Reference selection. Default HSI16 works on an UNMODIFIED NUCLEO-U575ZI-Q:
 *   0 = internal HSI16, no soldering (frequency NOT precise enough for FDMA)
 *   1 = ST-LINK-V3E 8 MHz to PH0, HSE bypass (solder bridges required)
 *   2 = 16 MHz HSE crystal X3, HSE oscillator mode (X3/C59/C60 required)
 */
#define TAG_CLOCK_REF_HSI16      0U
#define TAG_CLOCK_REF_STLINK8    1U
#define TAG_CLOCK_REF_XTAL16    2U
#define TAG_CLOCK_REF_SOURCE     TAG_CLOCK_REF_HSI16

#if (TAG_ID > 9U)
#error "TAG_ID must be an integer from 0 through 9"
#endif

#if (TAG_CLOCK_REF_SOURCE > TAG_CLOCK_REF_XTAL16)
#error "Unknown TAG_CLOCK_REF_SOURCE"
#endif

#endif /* TAG_CLOCK_CONFIG_H */

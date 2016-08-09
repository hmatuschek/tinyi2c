#ifndef __TEMP_H__
#define __TEMP_H__

#include <avr/io.h>

/** Initializes the ADC for temperature measurement. */
extern void temp_init();

/** Returns the external temperature in degree Celsius as a 16bit singed fixed point value.
 * That is, the MSB specifies the degree and the LSB the 265th faction of it. */
extern int16_t temp_get_ext();
/** Returns the internal temperature (ADC4) in degree Celsius as a 16bit singed fixed point value.
 * That is, the MSB specifies the degree and the LSB the 265th faction of it. */
extern int16_t temp_get_int();

/** Updates the temperature measurement with 64-fold oversampling. */
extern void temp_poll();

#endif // __TEMP_H__

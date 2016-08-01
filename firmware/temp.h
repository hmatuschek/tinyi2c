#ifndef __TEMP_H__
#define __TEMP_H__

#include <avr/io.h>

extern void temp_init();

extern int16_t temp_get_ext();
extern int16_t temp_get_int();
extern void temp_poll();

#endif // __TEMP_H__

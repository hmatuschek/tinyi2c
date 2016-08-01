#ifndef __LM90_H__
#define __LM90_H__

#include <avr/io.h>
#include <ctype.h>

#define LM90_ADDR 0x4C

extern uint8_t lm90_read(uint8_t *data, uint8_t len);
extern uint8_t lm90_write(uint8_t *data, uint8_t len);
extern uint8_t lm90_quick(uint8_t rd);
#define LM90_HOOK {LM90_ADDR, lm90_read, lm90_write, lm90_quick}
#endif

#ifndef __DS1621_H__
#define __DS1621_H__

#include <avr/io.h>
#include <ctype.h>

#define DS1621_ADDR 0x48

extern uint8_t ds1621_read(uint8_t *data, uint8_t len);
extern uint8_t ds1621_write(uint8_t *data, uint8_t len);
extern uint8_t ds1621_quick(uint8_t rd);

#define DS1621_HOOK {DS1621_ADDR, ds1621_read, ds1621_write, ds1621_quick}
#endif

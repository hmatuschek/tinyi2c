#ifndef __I2C_H__
#define __I2C_H__

#include <avr/io.h>

#define ENABLE_SCL_EXPAND

#define DEFAULT_DELAY 10  // default 10us (100khz)
extern unsigned short clock_delay;
extern unsigned short clock_delay2;

extern unsigned short expected;
extern unsigned char saved_cmd;


#define I2C_PORT   PORTB
#define I2C_PIN    PINB
#define I2C_DDR    DDRB
#define I2C_SDA    _BV(4)
#define I2C_SCL    _BV(0)

extern void i2c_init(void);
extern void i2c_start(void);
extern void i2c_repstart(void);
extern uint8_t i2c_put_u08(uint8_t b);
extern uint8_t i2c_get_u08(uint8_t last);
extern void i2c_stop(void);

#endif

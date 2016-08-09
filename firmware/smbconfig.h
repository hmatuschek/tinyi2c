/** @file smbconfig.h SMBus config file.
 * This file collects the definitions needed to set up the hardware I2C and virtual SMBus
 * interfaces. */

#ifndef __SMBCONFIG_H__
#define __SMBCONFIG_H__

// IO pins for the I2C HW interface
#define I2C_PORT   PORTB
#define I2C_PIN    PINB
#define I2C_DDR    DDRB
#define I2C_SDA    _BV(4)
#define I2C_SCL    _BV(0)

#include "lm90.h"
#include "ds1621.h"

/// Number of present SMBus hooks.
#define SMB_HOOK_COUNT 2
/// List of SMBus hooks.
#define SMB_HOOKS      LM90_HOOK, DS1621_HOOK

#endif // __SMBCONFIG_H__

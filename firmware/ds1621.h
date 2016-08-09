/** @file ds1621.h Implements a virtual DS1621 I2C temperature sensor.
 * This file provides the hook interface to the virtual DS1621 device. The device address is set
 * by the @c DS1621_ADDR macro and the temperature reading function with @c DS1621_GET_TEMP_FUNC.
 * The @c DS1621_HOOK macro can then be used to add the hook to the SMbus implementation in
 * @c smb.h
 *
 * @author Hannes Matuschek <hmatuschek@gmail.com>
 * @date 2016-08-02 */

#ifndef __DS1621_H__
#define __DS1621_H__

#include <avr/io.h>
#include <ctype.h>

/// Specifies the device address
#define DS1621_ADDR 0x48
/// Specifies the required buffer size
#define DS1621_REQ_BUFFER_SIZE 2

/** Specifies the function to call to get the current temperature value as a signed 16bit integer
 * in degree Celsius where the MSB specifies the degrees and the LSB the 256th
 * fraction of a degree. */
#define DS1621_GET_TEMP_FUNC temp_get_ext

/** The SMbus write() hook function.
 * This function processes all data send to the DS1621 device. */
extern uint8_t ds1621_write(uint8_t *data, uint8_t len);

/** The DS1621 virtual device hook. It uses the common SMBus send buffer via @c smb_read_buffer
 * and no SMBus "quick" message handler. */
#define DS1621_HOOK {DS1621_ADDR, smb_read_buffer, ds1621_write, NULL}

// Define or Update the required buffer size...
#ifndef SMB_OUTPUT_BUFFER_SIZE
// ... if not defined yet -> simply define
#define SMB_OUTPUT_BUFFER_SIZE DS1621_REQ_BUFFER_SIZE
#elif SMB_OUTPUT_BUFFER_SIZE < DS1621_REQ_BUFFER_SIZE
// ... if defined but too small -> redefine
#undef SMB_OUTPUT_BUFFER_SIZE
#define SMB_OUTPUT_BUFFER_SIZE DS1621_REQ_BUFFER_SIZE
#endif

#endif

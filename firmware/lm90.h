/** @file lm90.h Implements a virtual LM90 I2C temperature sensor.
 * This file provides the hook interface to the virtual LM90 device. The device address is set
 * by the @c LM90_ADDR macro and the temperature reading functions with @c LM90_GET_EXT_TEMP_FUNC
 * and @c LM90_GET_INT_TEMP_FUNC. The @c LM90_HOOK macro can then be used to add the hook to the
 * SMbus implementation in @c smb.h
 *
 * @author Hannes Matuschek <hmatuschek@gmail.com>
 * @date 2016-08-02 */

#ifndef __LM90_H__
#define __LM90_H__

#include <avr/io.h>
#include <ctype.h>

// Min output buffer size for LM90 responses
#define LM90_REQ_BUFFER_SIZE 1
// Default device address
#define LM90_ADDR 0x4C

/** Specifies the function to call to get the current external temperature value as a signed
 * 16bit integer in degree Celsius where the MSB specifies the degrees and the LSB the 256th
 * fraction of a degree. */
#define LM90_GET_EXT_TEMP_FUNC temp_get_ext
/** Specifies the function to call to get the current internal temperature value as a signed
 * 16bit integer in degree Celsius where the MSB specifies the degrees and the LSB the 256th
 * fraction of a degree. */
#define LM90_GET_INT_TEMP_FUNC temp_get_int

/** The SMB write() fuction implementation.
 * This function processes all data send to the LM90 device. */
extern uint8_t lm90_write(uint8_t *data, uint8_t len);

/** The LM90 virtual device hook. It uses the common SMBus send buffer via @c smb_read_buffer
 * and no SMBus "quick" message handler. */
#define LM90_HOOK {LM90_ADDR, smb_read_buffer, lm90_write, NULL}

// Define or update required send buffer size
#ifndef SMB_OUTPUT_BUFFER_SIZE
#define SMB_OUTPUT_BUFFER_SIZE LM90_REQ_BUFFER_SIZE
#elif SMB_OUTPUT_BUFFER_SIZE < LM90_REQ_BUFFER_SIZE
#undef SMB_OUTPUT_BUFFER_SIZE
#define SMB_OUTPUT_BUFFER_SIZE LM90_REQ_BUFFER_SIZE
#endif

#endif

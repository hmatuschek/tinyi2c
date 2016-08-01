#ifndef __LM90_H__
#define __LM90_H__

#include <avr/io.h>
#include <ctype.h>

// Min output buffer size for LM90 responses
#define LM90_REQ_BUFFER_SIZE 1
// Default device address
#define LM90_ADDR 0x4C

/** Specifies the function to call to get the current external temperature value as a signed
 * 16bit integer. */
#define LM90_GET_EXT_TEMP_FUNC temp_get_ext
/** Specifies the function to call to get the current internal temperature value as a signed
 * 16bit integer. */
#define LM90_GET_INT_TEMP_FUNC temp_get_int

// The SMB write() fuction implementation.
extern uint8_t lm90_write(uint8_t *data, uint8_t len);

#define LM90_HOOK {LM90_ADDR, smb_read_buffer, lm90_write, NULL}

#ifndef SMB_OUTPUT_BUFFER_SIZE
#define SMB_OUTPUT_BUFFER_SIZE LM90_REQ_BUFFER_SIZE
#elif SMB_OUTPUT_BUFFER_SIZE < LM90_REQ_BUFFER_SIZE
#undef SMB_OUTPUT_BUFFER_SIZE
#define SMB_OUTPUT_BUFFER_SIZE LM90_REQ_BUFFER_SIZE
#endif

#endif

#ifndef __DS1621_H__
#define __DS1621_H__

#include <avr/io.h>
#include <ctype.h>

/// Specifies the device address
#define DS1621_ADDR 0x48
/// Specifies the required buffer size
#define DS1621_REQ_BUFFER_SIZE 2

/// Specifies the function to call to get the current temperature value as a signed 16bit integer.
#define DS1621_GET_TEMP_FUNC temp_get_ext

/// The write() hook function
extern uint8_t ds1621_write(uint8_t *data, uint8_t len);

/// Assemble the hook struct
#define DS1621_HOOK {DS1621_ADDR, smb_read_buffer, ds1621_write, NULL}

// Update or define required buffer size...
#ifndef SMB_OUTPUT_BUFFER_SIZE
// ... if not defined yet -> simply define
#define SMB_OUTPUT_BUFFER_SIZE DS1621_REQ_BUFFER_SIZE
#elif SMB_OUTPUT_BUFFER_SIZE < DS1621_REQ_BUFFER_SIZE
// ... if defined but too small -> redefine
#undef SMB_OUTPUT_BUFFER_SIZE
#define SMB_OUTPUT_BUFFER_SIZE DS1621_REQ_BUFFER_SIZE
#endif

#endif

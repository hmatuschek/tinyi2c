#ifndef __SMB_H__
#define __SMB_H__

#include <inttypes.h>


/** Defines the SMBus hook structure. */
typedef struct {
  /** Address of the device. */
  uint8_t addr;
  /** Read callback function. */
  uint8_t (*read)(uint8_t *data, uint8_t len);
  /** Write callback function. First byte of data is SMB command. */
  uint8_t (*write)(uint8_t *data, uint8_t len);
  /** SMBus "quick" message handler. */
  uint8_t (*quick)(uint8_t rd);
} SMBHook;


/** Initializes the I2C & USB HW interface. */
extern void smb_init();

/** Puts a byte in the shared output buffer. */
extern void smb_put_byte(uint8_t b);

/** Puts a word in the shared output buffer (LSB first). */
extern void smb_put_word(uint16_t w);

/** Puts a word in the shared output buffer (MSB first). */
extern void smb_put_word_be(uint16_t w);

/** Default SMB read() implementation.
 * Just sends the shared output buffer. */
extern uint8_t smb_read_buffer(uint8_t *data, uint8_t len);

#endif // __SMB_H__

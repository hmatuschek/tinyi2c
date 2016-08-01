#include "proto.h"
#include "i2c.h"
#include "usbdrv/usbdrv.h"
#include "ds1621.h"
#include <string.h>


#ifndef PROGMEM
#define PROGMEM
#endif

#define min(a,b) (a<b) ? a : b

/* the currently support capability is quite limited */
const unsigned long func PROGMEM = I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;

static uint8_t status = STATUS_IDLE;

static SMBHook smb_hooks[SMB_HOOK_COUNT] = {SMB_HOOKS};
static SMBHook *smb_hook = 0;


static uint8_t
i2c_do(struct i2c_cmd *cmd) {
  uint8_t addr;

  /* normal 7bit address */
  addr = ( cmd->addr << 1 );
  if (cmd->flags & I2C_M_RD )
    addr |= 1;

  if(cmd->cmd & CMD_I2C_BEGIN)
    i2c_start();
  else
    i2c_repstart();

  // Clear hook on any new addr
  smb_hook = 0;

  // send DEVICE address (check if device is present)
  if (! i2c_put_u08(addr)) {
    // If device not found && no hook installed -> done
    status = STATUS_ADDRESS_NAK;
    expected = 0;
    i2c_stop();

    // If device is not present
    // -> check if device is simulated (e.g. there is a registered hook for it).
    for (uint8_t i=0; i<SMB_HOOK_COUNT; i++) {
      if (cmd->addr == smb_hooks[i].addr) {
        // Hook found ...
        smb_hook = &smb_hooks[i];

        // Ok
        status = STATUS_ADDRESS_ACK;
        expected = cmd->len;
        saved_cmd = cmd->cmd;

        // Handle SMBus "quick" messages
        if ((cmd->cmd & CMD_I2C_END) && (!expected) && smb_hook->quick) {
          smb_hook->quick(cmd->flags & I2C_M_RD);
        }
      }
    }
  } else {
    status = STATUS_ADDRESS_ACK;
    expected = cmd->len;
    saved_cmd = cmd->cmd;

    /* check if transfer is already done (or failed) */
    if ((cmd->cmd & CMD_I2C_END) && !expected)
      i2c_stop();
  }

  /* more data to be expected? */
  return ((((cmd->flags & I2C_M_RD) && cmd->len) || ! (cmd->flags & I2C_M_RD)) ? 0xff : 0x00);
}


/*---------------------------------------------------------------------------
 * usbFunctionRead
 *---------------------------------------------------------------------------*/
uint8_t usbFunctionRead( uint8_t *data, uint8_t len )
{
  uint8_t i;

  if (smb_hook) {
    uint8_t n = smb_hook->read(data, min(len, expected));
    expected -= n;
    while (expected) {
      data[n] = 0; n++; expected--;
    }
  } else if (status == STATUS_ADDRESS_ACK) {
    if(len > expected) {
      len = expected;
    }

    // consume bytes
    for(i=0;i<len;i++) {
      expected--;
      *data = i2c_get_u08(expected == 0);
      data++;
    }

    // end transfer on last byte
    if((saved_cmd & CMD_I2C_END) && !expected)
      i2c_stop();
  } else {
    memset(data, 0, len);
  }
  return len;
}


/*---------------------------------------------------------------------------
 * usbFunctionWrite
 *---------------------------------------------------------------------------*/
uint8_t usbFunctionWrite( uint8_t *data, uint8_t len )
{
  uint8_t i;

  if (smb_hook) {
    uint8_t n = smb_hook->write(data, min(len, expected));
    expected -= n;
    while (expected) {
      data[n] = 0; n++; expected--;
    }
  } else if (status == STATUS_ADDRESS_ACK) {
    if(len > expected) {
      len = expected;
    }

    // consume bytes
    for (i=0;i<len;i++) {
      expected--;
      i2c_put_u08(*data++);
    }

    // end transfer on last byte
    if (saved_cmd & CMD_I2C_END)
      i2c_stop();

  } else {
    memset(data, 0, len);
  }

  return len;
}


/*---------------------------------------------------------------------------
 * usbFunctionSetup
 *---------------------------------------------------------------------------*/
uint8_t	usbFunctionSetup(uint8_t data[8]) {
  static uint8_t replyBuf[4];
  usbMsgPtr = replyBuf;

  switch(data[1]) {

    case CMD_ECHO: // echo (for transfer reliability testing)
      replyBuf[0] = data[2];
      replyBuf[1] = data[3];
      return 2;

    case CMD_GET_FUNC:
      memcpy_P(replyBuf, &func, sizeof(func));
      return sizeof(func);

    case CMD_SET_DELAY:
      /* The delay function used delays 4 system ticks per cycle. */
      /* This gives 1/3us at 12Mhz per cycle. The delay function is */
      /* called twice per clock edge and thus four times per full cycle. */
      /* Thus it is called one time per edge with the full delay */
      /* value and one time with the half one. Resulting in */
      /* 2 * n * 1/3 + 2 * 1/2 n * 1/3 = n us. */
      clock_delay = *(unsigned short*)(data+2);
      if (!clock_delay)
        clock_delay = 1;
      clock_delay2 = clock_delay/2;
      if (!clock_delay2)
        clock_delay2 = 1;
      break;

    case CMD_I2C_IO:
    case CMD_I2C_IO + CMD_I2C_BEGIN:
    case CMD_I2C_IO                 + CMD_I2C_END:
    case CMD_I2C_IO + CMD_I2C_BEGIN + CMD_I2C_END:
      // these are only allowed as class transfers
      return i2c_do((struct i2c_cmd*)data);

    case CMD_GET_STATUS:
      replyBuf[0] = status;
      return 1;

    default:
      // must not happen ...
      break;
  }

  return 0;  // reply len
}



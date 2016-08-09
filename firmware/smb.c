#include "smb.h"
#include "smbconfig.h"
#include "usbdrv/usbdrv.h"
#include <util/delay.h>
#include <string.h>

// satisfy editor
#ifndef PROGMEM
#define PROGMEM
#endif


/* ********************************************************************************************* *
 * I2C Hardware interface
 * ********************************************************************************************* */
#define DEFAULT_DELAY 10  // default 10us (100khz)

static unsigned short clock_delay  = DEFAULT_DELAY;
static unsigned short clock_delay2 = DEFAULT_DELAY/2;

static void i2c_io_set_sda(uint8_t hi) {
  if(hi) {
    I2C_DDR  &= ~I2C_SDA;    // high -> input
    I2C_PORT |=  I2C_SDA;    // with pullup
  } else {
    I2C_DDR  |=  I2C_SDA;    // low -> output
    I2C_PORT &= ~I2C_SDA;    // drive low
  }
}

static uint8_t i2c_io_get_sda(void) {
  return(I2C_PIN & I2C_SDA);
}

static void i2c_io_set_scl(uint8_t hi) {
  _delay_loop_2(clock_delay2);
  if(hi) {
    I2C_DDR &= ~I2C_SCL;          // port is input
    I2C_PORT |= I2C_SCL;          // enable pullup

    // wait while pin is pulled low by client
    while(!(I2C_PIN & I2C_SCL));
  } else {
    I2C_DDR |= I2C_SCL;           // port is output
    I2C_PORT &= ~I2C_SCL;         // drive it low
  }
  _delay_loop_2(clock_delay);
}

static void i2c_init(void) {
  /* init the sda/scl pins */
  I2C_DDR &= ~I2C_SDA;            // port is input
  I2C_PORT |= I2C_SDA;            // enable pullup
  I2C_DDR &= ~I2C_SCL;            // port is input
  I2C_PORT |= I2C_SCL;            // enable pullup
}

/* clock HI, delay, then LO */
static void i2c_scl_toggle(void) {
  i2c_io_set_scl(1);
  i2c_io_set_scl(0);
}

/* i2c start condition */
static void i2c_start(void) {
  i2c_io_set_sda(0);
  i2c_io_set_scl(0);
}

/* i2c repeated start condition */
static void i2c_repstart(void)
{
  /* scl, sda may not be high */
  i2c_io_set_sda(1);
  i2c_io_set_scl(1);

  i2c_io_set_sda(0);
  i2c_io_set_scl(0);
}

/* i2c stop condition */
static void i2c_stop(void) {
  i2c_io_set_sda(0);
  i2c_io_set_scl(1);
  i2c_io_set_sda(1);
}

static uint8_t i2c_put_u08(uint8_t b) {
  char i;

  for (i=7;i>=0;i--) {
    if ( b & (1<<i) )  i2c_io_set_sda(1);
    else               i2c_io_set_sda(0);

    i2c_scl_toggle();           // clock HI, delay, then LO
  }

  i2c_io_set_sda(1);            // leave SDL HI
  i2c_io_set_scl(1);            // clock back up

  b = i2c_io_get_sda();         // get the ACK bit
  i2c_io_set_scl(0);            // not really ??

  return(b == 0);               // return ACK value
}

static uint8_t i2c_get_u08(uint8_t last) {
  char i;
  uint8_t c,b = 0;

  i2c_io_set_sda(1);            // make sure pullups are activated
  i2c_io_set_scl(0);            // clock LOW

  for(i=7;i>=0;i--) {
    i2c_io_set_scl(1);          // clock HI
    c = i2c_io_get_sda();
    b <<= 1;
    if(c) b |= 1;
    i2c_io_set_scl(0);          // clock LO
  }

  if(last) i2c_io_set_sda(1);   // set NAK
  else     i2c_io_set_sda(0);   // set ACK

  i2c_scl_toggle();             // clock pulse
  i2c_io_set_sda(1);            // leave with SDL HI

  return b;                     // return received byte
}

/* ********************************************************************************************* *
 * I2C-USB Software interface
 * ********************************************************************************************* */
/* commands from USB, must e.g. match command ids in kernel driver */
#define CMD_ECHO       0
#define CMD_GET_FUNC   1
#define CMD_SET_DELAY  2
#define CMD_GET_STATUS 3

#define CMD_I2C_IO     4
#define CMD_I2C_BEGIN  1  // flag fo I2C_IO
#define CMD_I2C_END    2  // flag fo I2C_IO

/* linux kernel flags */
#define I2C_M_TEN		0x10	/* we have a ten bit chip address */
#define I2C_M_RD		0x01
#define I2C_M_NOSTART		0x4000
#define I2C_M_REV_DIR_ADDR	0x2000
#define I2C_M_IGNORE_NAK	0x1000
#define I2C_M_NO_RD_ACK		0x0800

/* To determine what functionality is present */
#define I2C_FUNC_I2C			0x00000001
#define I2C_FUNC_10BIT_ADDR		0x00000002
#define I2C_FUNC_PROTOCOL_MANGLING	0x00000004 /* I2C_M_{REV_DIR_ADDR,NOSTART,..} */
#define I2C_FUNC_SMBUS_HWPEC_CALC	0x00000008 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_READ_WORD_DATA_PEC  0x00000800 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_WRITE_WORD_DATA_PEC 0x00001000 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_PROC_CALL_PEC	0x00002000 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_BLOCK_PROC_CALL_PEC 0x00004000 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_BLOCK_PROC_CALL	0x00008000 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_QUICK		0x00010000
#define I2C_FUNC_SMBUS_READ_BYTE	0x00020000
#define I2C_FUNC_SMBUS_WRITE_BYTE	0x00040000
#define I2C_FUNC_SMBUS_READ_BYTE_DATA	0x00080000
#define I2C_FUNC_SMBUS_WRITE_BYTE_DATA	0x00100000
#define I2C_FUNC_SMBUS_READ_WORD_DATA	0x00200000
#define I2C_FUNC_SMBUS_WRITE_WORD_DATA	0x00400000
#define I2C_FUNC_SMBUS_PROC_CALL	0x00800000
#define I2C_FUNC_SMBUS_READ_BLOCK_DATA	0x01000000
#define I2C_FUNC_SMBUS_WRITE_BLOCK_DATA 0x02000000
#define I2C_FUNC_SMBUS_READ_I2C_BLOCK	0x04000000 /* I2C-like block xfer  */
#define I2C_FUNC_SMBUS_WRITE_I2C_BLOCK	0x08000000 /* w/ 1-byte reg. addr. */
#define I2C_FUNC_SMBUS_READ_I2C_BLOCK_2	 0x10000000 /* I2C-like block xfer  */
#define I2C_FUNC_SMBUS_WRITE_I2C_BLOCK_2 0x20000000 /* w/ 2-byte reg. addr. */
#define I2C_FUNC_SMBUS_READ_BLOCK_DATA_PEC  0x40000000 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_WRITE_BLOCK_DATA_PEC 0x80000000 /* SMBus 2.0 */

#define I2C_FUNC_SMBUS_BYTE I2C_FUNC_SMBUS_READ_BYTE | \
                            I2C_FUNC_SMBUS_WRITE_BYTE
#define I2C_FUNC_SMBUS_BYTE_DATA I2C_FUNC_SMBUS_READ_BYTE_DATA | \
                                 I2C_FUNC_SMBUS_WRITE_BYTE_DATA
#define I2C_FUNC_SMBUS_WORD_DATA I2C_FUNC_SMBUS_READ_WORD_DATA | \
                                 I2C_FUNC_SMBUS_WRITE_WORD_DATA
#define I2C_FUNC_SMBUS_BLOCK_DATA I2C_FUNC_SMBUS_READ_BLOCK_DATA | \
                                  I2C_FUNC_SMBUS_WRITE_BLOCK_DATA
#define I2C_FUNC_SMBUS_I2C_BLOCK I2C_FUNC_SMBUS_READ_I2C_BLOCK | \
                                  I2C_FUNC_SMBUS_WRITE_I2C_BLOCK

#define I2C_FUNC_SMBUS_EMUL I2C_FUNC_SMBUS_QUICK | \
                            I2C_FUNC_SMBUS_BYTE | \
                            I2C_FUNC_SMBUS_BYTE_DATA | \
                            I2C_FUNC_SMBUS_WORD_DATA | \
                            I2C_FUNC_SMBUS_PROC_CALL | \
                            I2C_FUNC_SMBUS_WRITE_BLOCK_DATA | \
                            I2C_FUNC_SMBUS_WRITE_BLOCK_DATA_PEC | \
                            I2C_FUNC_SMBUS_I2C_BLOCK

struct i2c_cmd {
  unsigned char type;
  unsigned char cmd;
  unsigned short flags;
  unsigned short addr;
  unsigned short len;
};

#define STATUS_IDLE          0
#define STATUS_ADDRESS_ACK   1
#define STATUS_ADDRESS_NAK   2

#define min(a,b) (a<b) ? a : b

/* the currently support capability is quite limited */
const unsigned long func PROGMEM = I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;

static uint8_t  status = STATUS_IDLE;
static uint16_t expected = 0;
static uint8_t  saved_cmd = 0;

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
    // If device is not present...
    status = STATUS_ADDRESS_NAK;
    expected = 0;
    i2c_stop();

    // ... check if device is simulated (e.g. there is a registered hook for it).
    for (uint8_t i=0; i<SMB_HOOK_COUNT; i++) {
      if (cmd->addr == smb_hooks[i].addr) {
        // Hook found ...
        smb_hook = &smb_hooks[i];

        // Ok
        status = STATUS_ADDRESS_ACK;
        expected = cmd->len;
        saved_cmd = cmd->cmd;

        // Handle SMBus "quick" messages
        if ((cmd->cmd & CMD_I2C_END) && (! expected) && smb_hook->quick) {
          smb_hook->quick(cmd->flags & I2C_M_RD);
        }
      }
    }
  } else {
    // If device is present
    status = STATUS_ADDRESS_ACK;
    expected = cmd->len;
    saved_cmd = cmd->cmd;

    /* check if transfer is already done (or failed) */
    if ((cmd->cmd & CMD_I2C_END) && !expected)
      i2c_stop();
  }

  /* more data to be expected? */
  return ((((cmd->flags & I2C_M_RD) && cmd->len) || !((cmd->flags & I2C_M_RD) || (cmd->cmd & CMD_I2C_END))) ? 0xff : 0x00);
}


/*---------------------------------------------------------------------------
 * usbFunctionRead
 *---------------------------------------------------------------------------*/
uint8_t usbFunctionRead( uint8_t *data, uint8_t len )
{
  uint8_t i;

  if (smb_hook) {
    // If there is a hook installed -> read from hook
    len = smb_hook->read(data, min(len, expected));
    expected -= len;
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
    len = smb_hook->write(data, min(len, expected));
    expected -= len;
  } else if (status == STATUS_ADDRESS_ACK) {
    if(len > expected)
      len = expected;

    // consume bytes
    for (i=0;i<len;i++) {
      expected--;
      i2c_put_u08(*data++);
    }

    // end transfer on last byte
    if (saved_cmd & CMD_I2C_END)
      i2c_stop();
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


/* ********************************************************************************************* *
 * SMB<->USB interface setup
 * ********************************************************************************************* */
void smb_init() {
  i2c_init();

  /* clear usb ports */
  USB_CFG_IOPORT   &= (uchar)~((1<<USB_CFG_DMINUS_BIT)|(1<<USB_CFG_DPLUS_BIT));

  /* make usb data lines outputs */
  USBDDR    |= ((1<<USB_CFG_DMINUS_BIT)|(1<<USB_CFG_DPLUS_BIT));

  /* USB Reset by device only required on Watchdog Reset */
  _delay_loop_2(40000);   // 10ms

  /* make usb data lines inputs */
  USBDDR &= ~((1<<USB_CFG_DMINUS_BIT)|(1<<USB_CFG_DPLUS_BIT));

  usbInit();
}


/* ********************************************************************************************* *
 * Implementation of common SMB output buffer
 * ********************************************************************************************* */
static uint8_t smb_output_buffer[SMB_OUTPUT_BUFFER_SIZE];
static uint8_t smb_output_count = 0;

void
smb_put_byte(uint8_t b) {
  smb_output_buffer[0] = b;
  smb_output_count = 1;
}

uint8_t
smb_read_buffer(uint8_t *data, uint8_t len) {
  len = min(len, smb_output_count);
  memcpy(data, smb_output_buffer, len);
  return len;
}

#if SMB_OUTPUT_BUFFER_SIZE >= 2
void
smb_put_word(uint16_t w) {
  smb_output_buffer[0] = (w & 0xff);
  smb_output_buffer[1] = (w >> 8);
  smb_output_count = 2;
}

void
smb_put_word_be(uint16_t w) {
  smb_output_buffer[0] = (w >> 8);
  smb_output_buffer[1] = (w & 0xff);
  smb_output_count = 2;
}
#endif

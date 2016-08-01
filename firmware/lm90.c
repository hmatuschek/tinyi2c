#include "lm90.h"
#include "temp.h"
#include "string.h"

#define min(a,b) (a<b ? a : b)

#define READ_LTEMP        0x00  // initial: 0x00
#define READ_RTEMPH       0x01  // initial: 0x00
#define READ_STATUS_REG   0x02  // initial: 0x00
#define READ_CONFIG       0x03  // initial: 0x00
#define WRITE_CONFIG      0x09  // initial: ----
#define READ_CONV_RATE    0x04  // initial: 0x08
#define WRITE_CONV_RATE   0x0A  // initial: ----
#define READ_LHIGH        0x05  // initial: 0x46
#define WRITE_LHIGH       0x0B  // initial: ----
#define READ_LLOW         0x06  // initial: 0x00
#define WRITE_LLOW        0x0C  // initial: ----
#define READ_RHIGHH       0x07  // initial: 0x46
#define WRITE_RHIGHH      0x0D  // initial: ----
#define READ_RLOWH        0x08  // initial: 0x00
#define WRITE_RLOWH       0x0E  // initial: ----
#define WRITE_ONE_SHOT    0xF0  // initial: ----
#define READ_RTEMPL       0x10  // initial: 0x00
#define ACCESS_RTEMP_OFFH 0x11  // initial: 0x00
#define ACCESS_RTEMP_OFFL 0x12  // initial: 0x00
#define ACCESS_RHIGHL     0x13  // initial: 0x00
#define ACCESS_RLOWL      0x14  // initial: 0x00
#define ACCESS_RCRIT      0x19  // initial: 0x55
#define ACCESS_LCRIT      0x20  // initial: 0x55
#define ACCESS_CRIT_HYST  0x21  // initial: 0x0A
#define ACCESS_RDIOD_FLT  0xbf  // initial: 0x00
#define READ_MAN_ID       0xfe  // fixed: 0x01
#define READ_DEV_ID       0xff  // fixed: 0x21

uint8_t buffer_size = 0;
uint8_t buffer[4] = {0,0,0,0};

static void smb_put_byte(uint8_t b) {
  buffer[0] = b;
  buffer_size = 1;
}

/*static void smb_put_word(uint16_t w) {
  // Transmit LSB first
  buffer[0] = w & 0x00ff; buffer[1] = (w>>8);
  buffer_size = 2;
}*/


static uint8_t  reg_config      = 0x00;
static uint8_t  reg_conv_rate   = 0x00;
static uint8_t  reg_local_high  = 0x46;
static uint8_t  reg_local_low   = 0x00;
static uint16_t reg_remote_high = 0x4600;
static uint16_t reg_remote_low  = 0x4600;
static uint16_t reg_remote_off  = 0x0000;
static uint8_t  reg_local_crit  = 0x55;
static uint8_t  reg_remote_crit = 0x55;
static uint8_t  reg_crit_hyst   = 0x0a;
static uint8_t  reg_diode_flt   = 0x00;


uint8_t
lm90_read(uint8_t *data, uint8_t len) {
  len = min(buffer_size, len);
  memcpy(data, buffer, len);
  buffer_size -= len;
  return len;
}


uint8_t
lm90_write(uint8_t *data, uint8_t len) {
  if (! len) {
    return 0;
  }

  // Dispatch by command
  switch (data[0]) {
    case READ_LTEMP:
      smb_put_byte(temp_get_int() >> 8);
      break;
    case READ_RTEMPH:
      smb_put_byte(temp_get_ext() >> 8);
      break;
    case READ_STATUS_REG:
      smb_put_byte(0);
      break;
    case READ_CONFIG:
      smb_put_byte(reg_config);
      break;
    case WRITE_CONFIG:
      reg_config = data[1];
      break;
    case READ_CONV_RATE:
      smb_put_byte(reg_conv_rate);
      break;
    case WRITE_CONV_RATE:
      reg_conv_rate = data[1];
      break;
    case READ_LHIGH:
      smb_put_byte(reg_local_high);
      break;
    case WRITE_LHIGH:
      reg_local_high = data[1];
      break;
    case READ_LLOW:
      smb_put_byte(reg_local_low);
      break;
    case WRITE_LLOW:
      reg_local_low = buffer[0];
      break;
    case READ_RHIGHH:
      smb_put_byte(reg_remote_high >> 8);
      break;
    case WRITE_RHIGHH:
      reg_remote_high = (((uint16_t) data[1])<<8) + (reg_remote_high & 0x00ff);
      break;
    case READ_RLOWH:
      smb_put_byte(reg_remote_low >> 8);
      break;
    case WRITE_RLOWH:
      reg_remote_low = (((uint16_t) data[1])<<8) + (reg_remote_low & 0x00ff);
      break;
    case WRITE_ONE_SHOT:
      break; // skip one-shot command, measureing continiously anyway
    case READ_RTEMPL:
      smb_put_byte(temp_get_ext() & 0xff);
      break;
    case ACCESS_RTEMP_OFFH:
      if (2 == len)
        reg_remote_off = (((uint16_t)data[1])<<8) + (reg_remote_off & 0x00ff);
      else
        smb_put_byte(reg_remote_off>>8);
      break;
    case ACCESS_RTEMP_OFFL:
      if (2 == len)
        reg_remote_off = (reg_remote_off & 0xff00) + ((uint16_t)data[1]);
      else
        smb_put_byte(reg_remote_off & 0xff);
      break;
    case ACCESS_RHIGHL:
      if (2 == len)
        reg_remote_high = (reg_remote_high & 0xff00) + ((uint16_t)data[1]);
      else
        smb_put_byte(reg_remote_high & 0xff);
      break;
    case ACCESS_RLOWL:
      if (2 == len)
        reg_remote_low = (reg_remote_low & 0xff00) + ((uint16_t)data[1]);
      else
        smb_put_byte(reg_remote_low & 0xff);
      break;
    case ACCESS_LCRIT:
      if (2 == len)
        reg_local_crit = data[1];
      else
        smb_put_byte(reg_local_crit);
      break;
    case ACCESS_RCRIT:
      if (2 == len)
        reg_remote_crit = data[1];
      else
        smb_put_byte(reg_remote_crit);
      break;
    case ACCESS_CRIT_HYST:
      if (2 == len)
        reg_crit_hyst = data[1];
      else
        smb_put_byte(reg_crit_hyst);
      break;
    case ACCESS_RDIOD_FLT:
      if (2 == len)
        reg_diode_flt = data[1];
      else
        smb_put_byte(reg_diode_flt);
      break;
    case READ_MAN_ID:
      smb_put_byte(0x01);
      break;
    case READ_DEV_ID:
      smb_put_byte(0x21);
      break;

    default:
      break;
  }

  return len;
}

uint8_t
lm90_quick(uint8_t rd) {
  return 0x00;
}


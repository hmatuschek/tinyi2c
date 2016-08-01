#include "ds1621.h"
#include "temp.h"
#include "smb.h"

#include <string.h>

#define min(a,b) (a<b) ? a : b

#define READ_TEMP     0xaa
#define ACCESS_TH     0xa1
#define ACCESS_TL     0xa2
#define ACCESS_CONFIG 0xac
#define READ_COUNTER  0xa8
#define READ_SLOPE    0xa9
#define START_CONVERT 0xee
#define STOP_CONVERT  0x22

static uint8_t  reg_config    = 0x83;
static uint16_t reg_low_temp  = 0x1200;
static uint16_t reg_high_temp = 0x1b00;

extern int16_t DS1621_GET_TEMP_FUNC();

uint8_t
ds1621_write(uint8_t *data, uint8_t len)
{
  if (! len)
    return 0;

  switch (data[0]) {
    case READ_TEMP:
      smb_put_word_be(temp_get_ext());
      break;

    case ACCESS_TH:
      if (3 == len)
        reg_high_temp = ( (((int16_t)data[1])<<8) + data[2]);
      else
        smb_put_word_be(reg_high_temp);
      break;

    case ACCESS_TL:
      if (3 == len)
        reg_low_temp = ( (((int16_t)data[1])<<8) + data[2]);
      else
        smb_put_word_be(reg_low_temp);
      break;

    case ACCESS_CONFIG:
      if (2 == len)
        reg_config = data[1];
      else
        smb_put_byte(reg_config);
      break;

    case STOP_CONVERT:
    case START_CONVERT:
      // Ignored, sample continuously anyway
      break;

    case READ_COUNTER:
      smb_put_byte(0xff-(temp_get_ext() & 0xff));
      break;

    case READ_SLOPE:
      smb_put_byte(0xff);
      break;

    default:
      break;
  }

  return len;
}

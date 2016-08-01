#include "ds1621.h"
#include "proto.h"
#include "temp.h"

#include <string.h>

#define min(a,b) (a<b) ? a : b

#define CMD_READ_TEMP     0xaa
#define CMD_ACCESS_TH     0xa1
#define CMD_ACCESS_TL     0xa2
#define CMD_ACCESS_CONFIG 0xac
#define CMD_READ_COUNTER  0xa8
#define CMD_READ_SLOPE    0xa9
#define CMD_START_CONVERT 0xee
#define CMD_STOP_CONVERT  0x22

static uint8_t ds1621_bytes_available = 0;
static uint8_t ds1621_out_buffer[2];

static uint8_t configReg = 0x83;
static uint16_t currentTemp = 0x0000;
static uint16_t lowTemp = 0xc900;
static uint16_t highTemp = 0x7d00;


uint8_t ds1621_read(uint8_t *data, uint8_t len) {
  len = min(len, ds1621_bytes_available);
  memcpy(data, ds1621_out_buffer, len);
  ds1621_bytes_available -= len;
  return len;
}

uint8_t ds1621_write(uint8_t *data, uint8_t len) {
  if (! len) {
    return 0;
  }

  switch (data[0]) {
    case CMD_READ_TEMP:
      currentTemp = temp_get_ext();// & 0xff80;
      ds1621_out_buffer[0] = (currentTemp >> 8);
      ds1621_out_buffer[1] = (currentTemp & 0xff);
      ds1621_bytes_available = 2;
      return len;

    case CMD_ACCESS_TH:
      if (3 == len) {
        highTemp = ( (((int16_t)data[1])<<8) + data[2]);
        return 3;
      }
      ds1621_out_buffer[0] = (highTemp >> 8);
      ds1621_out_buffer[1] = (highTemp & 0xff);
      ds1621_bytes_available = 2;
      return len;

    case CMD_ACCESS_TL:
      if (3 == len) {
        lowTemp = ( (((int16_t)data[1])<<8) + data[2]);
        return 3;
      }
      ds1621_out_buffer[0] = (lowTemp >> 8);
      ds1621_out_buffer[1] = (lowTemp & 0xff);
      ds1621_bytes_available = 2;
      return len;

    case CMD_ACCESS_CONFIG:
      if (2 == len) {
        configReg = ( (configReg & 0xf0) | (data[1] & 0x03) );
        return 2;
      }
      ds1621_out_buffer[0] = configReg;
      ds1621_bytes_available = 1;
      return len;

    case CMD_STOP_CONVERT:
    case CMD_START_CONVERT:
      return len;

    case CMD_READ_COUNTER:
      ds1621_out_buffer[0] = 0xff-(currentTemp & 0xff);
      ds1621_bytes_available = 1;
      return len;

    case CMD_READ_SLOPE:
      ds1621_out_buffer[0] = 0xff;
      ds1621_bytes_available = 1;
      return len;

    default:
      break;
  }

  return len;
}

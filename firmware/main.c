/* Name: main.c
 * Project: i2c-tiny-usb
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2005 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: main.c,v 1.9 2007/06/07 13:53:47 harbaum Exp $
 *
 * $Log: main.c,v $
 * Revision 1.9  2007/06/07 13:53:47  harbaum
 * Version number fixes
 *
 * Revision 1.8  2007/05/19 12:30:11  harbaum
 * Updated USB stacks
 *
 * Revision 1.7  2007/04/22 10:34:05  harbaum
 * *** empty log message ***
 *
 * Revision 1.6  2007/01/05 19:30:58  harbaum
 * i2c clock bug fix
 *
 * Revision 1.5  2007/01/03 18:35:07  harbaum
 * usbtiny fixes and pcb layouts
 *
 * Revision 1.4  2006/12/03 21:28:59  harbaum
 * *** empty log message ***
 *
 * Revision 1.3  2006/11/22 19:12:45  harbaum
 * Added usbtiny support
 *
 * Revision 1.2  2006/11/14 19:15:13  harbaum
 * *** empty log message ***
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
//#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <util/delay.h>

// use avrusb library
#include "usbdrv/usbdrv.h"
#include "i2c.h"
#include "ds1621.h"
#include "temp.h"


/* ------------------------------------------------------------------------- */

int	main(void) {
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
  temp_init();

  sei();
  for(;;) {	/* main event loop */
    //wdt_reset();
    usbPoll();
  }

  return 0;
}

/* ------------------------------------------------------------------------- */

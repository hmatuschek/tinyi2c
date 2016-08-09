# tiny-i2c -- An augmented i2c-tiny-usb compatible device emulating various I2C devices.

This tiny USB device provides an I2C interface based on the original [i2c-tiny-usb](http://www.harbaum.org/till/i2c_tiny_usb/index.shtml) device. Hence, it is supported by the Linux kernel. Additionally, it is able to emulate various I2C devices in firmware. For example, it is able to emulate the [LM90](http://www.ti.com/product/LM90) temperature sensor by using a simple NTC resistor as a temperature sensor and the build-in temperature sensor of the [ATTiny45](http://www.atmel.com/images/atmel-2586-avr-8-bit-microcontroller-attiny25-attiny45-attiny85_datasheet.pdf).

## License
tinyi2c - Copyright (C) 2016 Hannes Matuschek

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

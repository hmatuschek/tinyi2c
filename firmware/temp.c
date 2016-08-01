#include "temp.h"
#include <avr/interrupt.h>


#ifndef ISR
#define ISR(x) void x()
#endif

volatile uint16_t tempSum;
volatile uint16_t currentExtTemp;
volatile uint16_t currentLocTemp;
uint8_t sumCount;


void temp_init() {
  DDRB  &= ~(1<<DDB3);
  PORTB &= ~(1<<DDB3);
  DIDR0 |=  (1<<ADC3D);

  // Config ADC (Vcc ref, right adjust, selected pin)
  ADMUX   = (0<<REFS2) | (0<<REFS1) | (0<<REFS0) | (0<<ADLAR) | 0x03;
  // config prescaler to 128
  ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
  // enable ADC
  ADCSRA |= (1<<ADEN);

  // Config timer 0
  TCCR0A = 0;
  TCCR0B = (1<<CS02) | (0<<CS01) | (1<<CS00);
  TIMSK |= (1<<TOIE0);

  tempSum    = 0;
  currentExtTemp = 0;
  currentLocTemp = 0;
  sumCount   = 0;
}


int16_t
temp_get_ext() {
  return ((((int32_t)currentExtTemp)*98) >> 8) - 7302;
}

int16_t
temp_get_int() {
  return ((((int32_t)currentLocTemp)*98) >> 8) - 7302;
}


//ISR(TIMER0_OVF_vect) {
void temp_poll() {
  // If still converting -> skip
  if (ADCSRA & (1<<ADSC))
    return;

  // if ready -> add value to sum
  tempSum += ADC;
  // restart conversion
  ADCSRA |= (1<<ADSC);

  // increment sum counter
  sumCount++;
  if (64 == sumCount) {
    // Once complete
    currentExtTemp = tempSum;
    tempSum = 0; sumCount = 0;
  }
}

ISR(TIMER0_OVF_vect) {
  temp_poll();
}

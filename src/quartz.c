#include <avr/io.h>
#include <avr/interrupt.h>
#include "tiny-midi-clock/midi_clock.h"

#define LED PD6

// Snagged from Lady Ada's blog post
#define output_low(port,pin) port &= ~(1<<pin)
#define output_high(port,pin) port |= (1<<pin)
#define set_input(portdir,pin) portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |= (1<<pin)

// Lifted from an avr tutorial
#define CLK_HIGH()  PORTB |= (1<<PB7)
#define CLK_LOW()   PORTB &= ~(1<<PB7)
#define CS_HIGH()   PORTB |= (1<<PB5)
#define CS_LOW()    PORTB &= ~(1<<PB5)
#define DATA_HIGH() PORTB |= (1<<PB0)
#define DATA_LOW()  PORTB &= ~(1<<PB0)
#define INIT_PORT() DDRB |= (1<<PB0) | (1<<PB5) | (1<<PB7)

#ifndef F_CPU
#define F_CPU 20000000UL
#endif

// Default: 120.0 BPM
volatile float bpm = 120.0; 

volatile uint32_t pulse_count = 0;
volatile char toggle = 0;

void toggle_led();
void setup_timer_interrupt();
void setup();

void setup() {
  set_output(DDRD, LED);
  setup_timer_interrupt();
  max7219_init();
}

void setup_timer_interrupt() {
  OCR0A = F_CPU / 1000000; // Sets the Output Compare Register (OCR) to count every microsecond.

  // Took from startingelectronics.com tutorial
  TCCR0A = 0x02;          // Clear Timer on Compare Match (CTC) mode
  TIFR |= 0x01;           // Reset the Timer flag.
  TIMSK = 0x01;           // TC0 compare match A interrupt enable
  TCCR0B = (1 << CS00);   // Select no prescaler for timer interrupt
  
  CLKPR = (1 << CLKPS0);  // Select a /2 prescaler for system clock so that LED timer gets 10Mhz
  
  sei();                  // enable global interrupt handler
}

ISR(TIMER0_COMPA_vect) {
  pulse_count++;
  uint32_t uc = (microseconds_per_pulse(bpm) / 192);

  if(pulse_count >= uc) {
      toggle_led();
      pulse_count = 0;
  }
}

void toggle_led() {
  if(toggle) {
    toggle = 0;
    output_high(PORTD, LED);      
  } else {
    toggle = 1;
    output_low(PORTD, LED);    
  }
}

int main(void) {
  setup();
  
   while (1) {
     // no-op, main loop.
   }
}



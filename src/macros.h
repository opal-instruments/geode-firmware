#define CV_CLK PB4
#define CV_CLK_HIGH()          PORTB |= (1 << CV_CLK)
#define CV_CLK_LOW()           PORTB &= ~(1 << CV_CLK)

#define CV_DATA PB3
#define CV_DATA_HIGH()          PORTB |= (1 << CV_DATA)
#define CV_DATA_LOW()           PORTB &= ~(1 << CV_DATA)

#define CV_LATCH PB2
#define CV_LATCH_HIGH()         PORTB |= (1 << CV_LATCH)
#define CV_LATCH_LOW()          PORTB &= ~(1 << CV_LATCH)

#define DISPLAY_CLK PB1
#define DISPLAY_CLK_HIGH()      PORTB |= (1 << DISPLAY_CLK)
#define DISPLAY_CLK_LOW()       PORTB &= ~(1 << DISPLAY_CLK)

#define DISPLAY_DATA PB0
#define DISPLAY_DATA_HIGH()     PORTB |= (1 << DISPLAY_DATA)
#define DISPLAY_DATA_LOW()      PORTB &= ~(1 << DISPLAY_DATA)

#define DISPLAY_LATCH PD6
#define DISPLAY_LATCH_HIGH()    PORTD |= (1 << DISPLAY_LATCH)
#define DISPLAY_LATCH_LOW()     PORTD &= ~(1 << DISPLAY_LATCH)

#define INIT_PORTB() DDRB |= (1 << CV_LATCH) | (1 << CV_CLK) | (1 << CV_DATA) | (1 << DISPLAY_CLK) | (1 << DISPLAY_DATA)
#define INIT_PORTD() DDRD |= (1 << DISPLAY_LATCH)

#define ROTARY_PHASE_A PB1
#define ROTARY_PHASE_B PB0
#define INIT_ROTARY() PORTB |= (1 << ROTARY_PHASE_A) | (1 << ROTARY_PHASE_B)
#define INIT_PCI_ROTARY() PCMSK |= (1 << ROTARY_PHASE_A) | (1 << ROTARY_PHASE_B)
#define INIT_PCI() GIMSK |= (1 << PCIE);
#define ROTARY_PIN(rotary, pin) ((rotary >> pin) & 0x01)

#ifndef F_CPU
#define F_CPU 20000000UL
#endif

// midi constants
#define CLOCK 0xF8
#define START 0xFA
#define CONTINUE 0xFB
#define STOP 0xFC
#define MIDI_CLOCK_PRECISION 24
#define MIDI_BEATS_PER_MEASURE 4

// MIDI messages are transmitted serially at 31.25 kbit/s.
#define MIDI_BAUD_RATE 31250
#define MINUTE 60
#define MICROSECOND 1000000

#define THOUSAND 1000
#define HUNDRED 100
#define TEN 10

// USART constants
#define UBRRH_PADDING 8
#define UBRRL_MASK 255
#define F_CPU_BAUD_RATE 40

// SPI Constants
#define MSB_16_BIT_HIGH 0x8000

// Seven Segment Display Constants
#define CHAR0 0x3F
#define CHAR1 0x06
#define CHAR2 0x5B
#define CHAR3 0x4F
#define CHAR4 0x66
#define CHAR5 0x6D
#define CHAR6 0x7D
#define CHAR7 0x07
#define CHAR8 0x7F
#define CHAR9 0x6F
#define CHAR_DOT 0x80
#define CHAR_NONE 0x00
#define CHAR_LED_HIGH 0x01

#define DIGIT1 0x01
#define DIGIT2 0x02
#define DIGIT3 0x04
#define DIGIT4 0x08
#define DIGIT_LED 0x80 // we reserve last bit of shift register for led
#define DIGIT_NONE 0x00

#define LED_DELAY 5
#define CHAR_OFFSET 4

// Rotary constants
#define ROTARY_LEFT 1
#define ROTARY_RIGHT 2
#define ROTARY_NONE 0

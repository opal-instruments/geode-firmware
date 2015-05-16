#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#define LED PD6
#define INIT_LED() DDRD |= (1 << LED)
#define LED_HIGH() PORTD |= (1 << LED)
#define LED_LOW()  PORTD &= ~(1 << LED)

#define CLK PB4
#define CLK_HIGH()  PORTB |= (1 << CLK)
#define CLK_LOW()   PORTB &= ~(1 << CLK)

#define MOSI PB3
#define MOSI_HIGH() PORTB |= (1 << MOSI)
#define MOSI_LOW()  PORTB &= ~(1 << MOSI)

#define LATCH PB2
#define LATCH_HIGH()   PORTB |= (1 << LATCH)
#define LATCH_LOW()    PORTB &= ~(1 << LATCH)

#define INIT_PORT() DDRB |= (1 << CLK) | (1 << MOSI) | (1 << LATCH)

#define MIDI_SERIAL PB1
#define INIT_MIDI_SERIAL() DDRB |= (1 << MIDI_SERIAL)

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

// USART constants
#define UBRRH_PADDING 8

// Default: 120.0 BPM
volatile float bpm = 120.0;
volatile int led = 0;
volatile uint16_t uc = 0;
volatile uint16_t toggle_count = 0;

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
#define CHARDOT 0x80
#define CHAR_NONE 0x00
#define CHAR_LED_HIGH 0x01

#define DIGIT1 0x01
#define DIGIT2 0x02
#define DIGIT3 0x04
#define DIGIT4 0x08
#define DIGIT_LED 0x80 // we reserve last bit of shift register for led
#define DIGIT_NONE 0x00

#define LED_DELAY 5
#define CHAR_OFFSET 8

uint32_t microseconds_per_pulse(float bpm);

void toggle_led();
void setup_timer();
void setup();
void spi_send(uint16_t data);
void write_char_at_digit(uint8_t character, uint8_t digit);
void usart_init();

void spi_send(uint16_t data) {
    uint8_t i;

    LATCH_LOW();

    for (i = 0; i < 16; i++) {
        if (data & 0X8000) {
            MOSI_HIGH();
        } else {
            MOSI_LOW();
        }

        CLK_LOW();
        CLK_HIGH();

        data <<= 1;
    }

    LATCH_HIGH();
}

// Returns the number of microseconds that transpire
// for each MIDI CLOCK pulse for the passed in bpm.
uint32_t microseconds_per_pulse(float bpm) {
    return ((MINUTE * MICROSECOND) / bpm);
}

void usart_init() {
    // Set baud rate
    UBRRH = MIDI_BAUD_RATE >> UBRRH_PADDING;
    UBRRL = MIDI_BAUD_RATE;

    // Enable TX
    UCSRB = (1 << TXEN);

    // A start bit and a stop bit are added to each byte for framing purposes,
    // so a MIDI byte requires ten bits for transmission.
    UCSRC = (1 << UCSZ1) | (1 << UCSZ0);

}

void setup() {
    INIT_PORT();
    INIT_LED();
    usart_init();

    LED_HIGH();
    LATCH_HIGH();
    CLK_HIGH();

    uc = microseconds_per_pulse(bpm);

    setup_timer();
}

void setup_timer() {
    // Sets the 16-bit output compare register
    // to trigger for every MIDI beat pulse
    OCR1A = uc * (F_CPU / MICROSECOND);

    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS10);

    TIMSK = 1 << OCIE1A;
    sei();
}

void toggle_led() {
    if(led == 1) {
        led = 0;
    } else {
        led = 1;
    }
}

ISR(TIMER1_COMPA_vect) {
    // If we have a match, clear the Flag register
    // and toggle the led.
    toggle_count++;
    if(toggle_count >= MIDI_CLOCK_PRECISION * MIDI_BEATS_PER_MEASURE) {
        toggle_count = 0;
        toggle_led();
    }
}

void draw_led() {
    if (led == 1) {
        write_char_at_digit(CHAR_LED_HIGH, DIGIT_LED);
    } else {
        write_char_at_digit(CHAR_NONE, DIGIT_NONE);
    }
}

void write_char_at_digit(uint8_t character, uint8_t digit) {
    spi_send(character << CHAR_OFFSET | digit);
    _delay_ms(LED_DELAY);
}

int main(void) {
    setup();

    while (1) {
        write_char_at_digit(CHAR1, DIGIT1);
        write_char_at_digit(CHAR2, DIGIT2);
        write_char_at_digit(CHAR0 | CHARDOT, DIGIT3);
        write_char_at_digit(CHAR0, DIGIT4);
        draw_led();
    }
}

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

// Default: 120.0 BPM
volatile float bpm = 120.0;
volatile uint8_t rotary_current = 0;
volatile uint8_t rotary_last = 0;
volatile uint8_t rotary_mask = 0;
volatile uint8_t rotary_direction = 0;
volatile int led = 0;
volatile uint16_t uc = 0;
volatile uint16_t toggle_count = 0;
volatile int pulse = 0;

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

const uint8_t characters[] = {
    CHAR0, CHAR1, CHAR2, CHAR3,
    CHAR4, CHAR5, CHAR6, CHAR7,
    CHAR8, CHAR9, CHAR_DOT, CHAR_NONE,
    CHAR_LED_HIGH
};

#define DIGIT1 0x01
#define DIGIT2 0x02
#define DIGIT3 0x04
#define DIGIT4 0x08
#define DIGIT_LED 0x80 // we reserve last bit of shift register for led
#define DIGIT_NONE 0x00

#define LED_DELAY 5
#define CHAR_OFFSET 8

// Rotary constants
#define ROTARY_LEFT 1
#define ROTARY_RIGHT 2
#define ROTARY_NONE 0

uint32_t microseconds_per_pulse(float bpm);

void toggle_led();
void setup_timer();
void setup();
void spi_send(uint16_t data);
void write_char_at_digit(uint8_t character, uint8_t digit);
void write_midi(uint8_t command, uint8_t data);
void usart_init();
void start_midi();
void clock_midi();
void stop_midi();
void send_midi();
void write_uart(uint8_t character);

void spi_send(uint16_t data) {
    uint8_t i;

    LATCH_LOW();

    for (i = 0; i < 16; i++) {
        if (data & MSB_16_BIT_HIGH) {
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
    UBRRH = F_CPU_BAUD_RATE >> UBRRH_PADDING;
    UBRRL = F_CPU_BAUD_RATE &  UBRRL_MASK;

    // Enable TX
    UCSRB = (1 << TXEN);

    // A start bit and a stop bit are added to each byte for framing purposes,
    // so a MIDI byte requires ten bits for transmission.
    UCSRC = (1 << UCSZ1) | (1 << UCSZ0);
}

void setup() {
    INIT_PORT();
    INIT_LED();
    INIT_ROTARY();
    INIT_PCI_ROTARY();
    INIT_PCI();
    usart_init();

    LED_HIGH();
    LATCH_HIGH();
    CLK_HIGH();

    uc = microseconds_per_pulse(bpm);

    setup_timer();

    start_midi();
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
    pulse++;

    if(toggle_count >= MIDI_CLOCK_PRECISION * MIDI_BEATS_PER_MEASURE) {
        toggle_count = 0;
        toggle_led();
    }
}

ISR(PCINT_vect) {

    // Grab current snapshot of PINB registers
    rotary_current = PINB;

    // XOR the current PINB registers
    // with what we had last.
    // Example:
    //
    // 00000000
    // 00100000
    //      XOR
    // --------
    // 00100000
    //
    // This indicates that there was a change
    // on an analog pin.
    //
    // For active pins, this can also be used
    // to observe no change.
    //
    // 00100000
    // 00100000
    //      XOR
    // --------
    // 00000000
    //
    // We will use this value to understand
    // which direction the knob is being turned.
    rotary_mask = rotary_current ^ rotary_last;

    // finally, set our last known value to what's current.
    rotary_last = rotary_current;
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

void write_midi(uint8_t command, uint8_t data) {
    write_uart(command);
    write_uart(data);
}

void write_uart(uint8_t character) {
    while(!(UCSRA & (1 << UDRE))) {}
    UDR = character;
}

void send_midi() {
    if(pulse > 0) {
        clock_midi();
        pulse = 0;
    }
}

void clock_midi() {
    write_midi(CLOCK, 0x00);
}

void stop_midi() {
    write_midi(STOP, 0x00);
}

void start_midi() {
    write_midi(START, 0x00);
}

void set_bpm() {

    // if the rotary encoder was previously stopped
    //   if it moved left
    //     set left
    //   else if it moved right
    //     set right
    if(ROTARY_PIN(rotary_current, ROTARY_PHASE_A) == 0 &&
            ROTARY_PIN(rotary_current, ROTARY_PHASE_B) == 0) {

        if(ROTARY_PIN(rotary_mask, ROTARY_PHASE_A) == 1) {
            rotary_direction = ROTARY_LEFT;
        } else if (ROTARY_PIN(rotary_mask, ROTARY_PHASE_B) == 1) {
            rotary_direction = ROTARY_RIGHT;
        } else {
            rotary_direction = ROTARY_NONE;
        }
    } else {
        if((rotary_direction == ROTARY_LEFT && ROTARY_PIN(rotary_mask, ROTARY_PHASE_A) == 0) ||
                (rotary_direction == ROTARY_RIGHT && ROTARY_PIN(rotary_mask, ROTARY_PHASE_B) == 0)) {
            rotary_direction = ROTARY_NONE;
        }
    }

    rotary_mask = 0;

    if(rotary_direction == ROTARY_LEFT) {
        bpm += 1;
    } else if(rotary_direction == ROTARY_RIGHT) {
        bpm -= 1;
    }
}

// TODO: Improve so that it's not so iterative; can be applied to window
void draw_display() {
    uint16_t tmp = (uint16_t) (bpm * 10);
    write_char_at_digit(characters[(tmp/THOUSAND)], DIGIT1);
    tmp %= THOUSAND;
    write_char_at_digit(characters[(tmp/HUNDRED)], DIGIT2);
    tmp %= HUNDRED;
    write_char_at_digit(characters[(tmp/TEN)] | CHAR_DOT, DIGIT3);
    tmp %= TEN;
    write_char_at_digit(characters[tmp], DIGIT4);
}

int main(void) {
    setup();

    while (1) {
        draw_display();
        draw_led();
        send_midi();

        if(rotary_mask > 0) {
            set_bpm();
        }
    }
}

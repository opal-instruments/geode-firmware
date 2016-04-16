#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include "./macros.h"

// Default: 120.0 BPM
volatile float bpm = 120.0;
volatile uint8_t rotary_current = 0;
volatile uint8_t rotary_last = 0;
volatile uint8_t rotary_mask = 0;
volatile uint8_t rotary_direction = 0;
volatile uint16_t uc = 0;
volatile uint16_t toggle_count = 0;
volatile int pulse = 0;
volatile uint8_t div = 0b00000000;

const uint8_t characters[] = {
    CHAR0, CHAR1, CHAR2, CHAR3,
    CHAR4, CHAR5, CHAR6, CHAR7,
    CHAR8, CHAR9, CHAR_DOT, CHAR_NONE,
    CHAR_LED_HIGH
};

uint32_t microseconds_per_pulse(float bpm);
void setup_timer();
void setup();
void display_spi_send(uint16_t data);
void cv_spi_send(uint16_t data);
void write_char_at_digit(uint8_t character, uint8_t digit);
void write_midi(uint8_t command, uint8_t data);
void usart_init();
void start_midi();
void clock_midi();
void stop_midi();
void send_midi();
void write_uart(uint8_t character);
void write_cv(uint16_t data);

void display_spi_send(uint16_t data) {
    uint8_t i;

    DISPLAY_LATCH_LOW();

    for (i = 0; i < 16; i++) {
        if (data & MSB_16_BIT_HIGH) {
            DISPLAY_DATA_HIGH();
        } else {
            DISPLAY_DATA_LOW();
        }

        DISPLAY_CLK_LOW();
        DISPLAY_CLK_HIGH();

        data <<= 1;
    }

    DISPLAY_LATCH_HIGH();
}

void cv_spi_send(uint16_t data) {
    uint8_t i;

    CV_LATCH_LOW();

    for (i = 0; i < 16; i++) {
        if (data & MSB_16_BIT_HIGH) {
            CV_DATA_HIGH();
        } else {
            CV_DATA_LOW();
        }

        CV_CLK_LOW();
        CV_CLK_HIGH();

        data <<= 1;
    }

    CV_LATCH_HIGH();
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
    INIT_PORTB();
    INIT_PORTD();
    //INIT_ROTARY();
    //INIT_PCI_ROTARY();
    //INIT_PCI();
    //usart_init();

    DISPLAY_LATCH_HIGH();
    CV_LATCH_HIGH();
    DISPLAY_CLK_HIGH();
    CV_CLK_HIGH();

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

ISR(TIMER1_COMPA_vect) {
    // If we have a match, clear the Flag register
    // and toggle the led.
    toggle_count++;
    pulse++;

    if(toggle_count >= MIDI_CLOCK_PRECISION * MIDI_BEATS_PER_MEASURE) {
        toggle_count = 0;
        write_cv(0x00);
    } else {
        write_cv(0x01);
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

void write_char_at_digit(uint8_t character, uint8_t digit) {
    display_spi_send(character << CHAR_OFFSET | digit);
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

void write_cv(uint16_t data) {
    cv_spi_send(data);
    _delay_ms(LED_DELAY);
}

void clock_midi() {
    write_midi(CLOCK, 0x00);
}

void stop_midi() {
    write_midi(STOP, 0x00);
}

void start_midi() {
    write_midi(START, 0x00);
    write_midi(CLOCK, 0x00);
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
        //draw_led();
        //send_midi();
        //if(rotary_mask > 0) {
        //    set_bpm();
        //}
    }
}

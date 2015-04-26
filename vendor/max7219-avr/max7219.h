#ifndef _MAX7219_H
#define _MAX7219_H

#include <stdint.h>
#include <avr/io.h>

#define DECODE_MODE 0x09
#define INTENSITY 0x0A
#define SCAN_LIMIT 0x0B
#define SHUTDOWN_REGISTER 0x0C
#define DISPLAY_TEST 0x0F

typedef struct _max7219_driver_t {
  int clk;
  int cs;
  int data;
  
  int portb;
  int ddrb;
} max7219_driver_t;

void max7219_writec(max7219_driver_t *drv, uint8_t high_byte, uint8_t low_byte);
void max7219_clear(max7219_driver_t *drv);
void max7219_new(int clk, int cs, int data);
void max7219_init(max7219_driver_t *drv);
void max7219_put_setting(max7219_driver_t *drv, uint8_t mode, uint8_t val);

void clk_high(max7219_driver_t *drv);
void clk_low(max7219_driver_t *drv);
void cs_high(max7219_driver_t *drv);
void cs_low(max7219_driver_t *drv);
void data_high(max7219_driver_t *drv);
void data_low(max7219_driver_t *drv);
void init_port(max7219_driver_t *drv);
void spi_send(max7219_driver_t *drv, uint8_t data);

#endif

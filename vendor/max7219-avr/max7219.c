#include "max7219.h"

void spi_send(max7219_driver_t *drv, uint8_t data) {
  uint8_t i;

  for (i = 0; i < 8; i++, data <<= 1) {
    clk_low(drv);
    if (data & 0x80) {
      data_high(drv);
    } else{
      data_low(drv);
    }
    clk_high(drv);
  }

}

void max7219_writec(max7219_driver_t *drv, uint8_t high_byte, uint8_t low_byte) {
  cs_low(drv);
  spi_send(drv, high_byte);
  spi_send(drv, low_byte);
  cs_high(drv);
}

void max7219_clear(max7219_driver_t *drv) {
  uint8_t i;
  for (i = 0; i < 8; i++) {
    max7219_writec(drv, i+1, 0);
  }
}

max7219_driver_t * max7219_new(int clk, int cs, int data, int portb, int ddrb) {
  max7219_driver_t *drv = malloc(sizeof(max7219_driver_t));
  drv->clk   = clk;
  drv->cs    = cs;
  drv->data  = data;
  drv->portb = portb;
  drv->ddrb  = ddrb;
  return drv;
}

// TODO: Have a way if specifying 
void max7219_init(max7219_driver_t *drv) {
  init_port(drv);

  // Decode mode: none
  max7219_put_setting(drv, DECODE_MODE, 0);

  // Intensity: 3 (0-15)
  max7219_put_setting(drv, INTENSITY, 1);

  // Scan limit: All "digits" (rows) on
  max7219_put_setting(drv, SCAN_LIMIT, 7);

  // Shutdown register: Display on
  max7219_put_setting(drv, SHUTDOWN_REGISTER, 1);

  // Display test: off
  max7219_put_setting(drv, DISPLAY_TEST, 1);

  max7219_clear(drv);
}
  
void max7219_put_setting(max7219_driver_t *drv, uint8_t mode, uint8_t val) {
  max7219_writec(drv, mode, val);
}

void clk_high(max7219_driver_t *drv) {
  drv->portb |= (1<<drv->clk);
}

void clk_low(max7219_driver_t *drv) {
  drv->portb &= ~(1<<drv->clk);
}

void cs_high(max7219_driver_t *drv) {
  drv->portb |= (1<<drv->cs);
}

void cs_low(max7219_driver_t *drv) {
  drv->portb &= ~(1<<drv->cs);
}

void data_high(max7219_driver_t *drv) {
  drv->portb |= (1<<drv->data);
}

void data_low(max7219_driver_t *drv) {
  drv->portb &= ~(1<<drv->data);
}

void init_port(max7219_driver_t *drv) {
  drv->ddrb |= (1<< drv->data) | (1<< drv->cs) | (1<< drv->data);
}

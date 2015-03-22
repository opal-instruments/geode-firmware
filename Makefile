CC=avr-gcc
OBJCOPY=avr-objcopy

all: clean
	mkdir bin
	$(CC) src/quartz.c -I./deps -mmcu=attiny2313 -o bin/quartz.o
	$(OBJCOPY) -O ihex -R .eeprom bin/quartz.o bin/quartz.hex

clean:
	rm -rf bin

upload:
	sudo avrdude -p attiny2313 -c avrisp2 -U lfuse:w:0xFF:m -U hfuse:w:0xFF:m -U efuse:w:0xFF:m -U flash:w:bin/quartz.hex:i -b 2400 

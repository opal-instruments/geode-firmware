CC=avr-gcc
OBJCOPY=avr-objcopy
LIB_DIR=./vendor/libs/
LIBS=-static -ltiny-midi-clock

all: clean deps
	mkdir -p bin
	$(CC) src/quartz.c -I./vendor -L$(LIB_DIR) $(LIBS) -mmcu=attiny2313 -o bin/quartz.o
	$(OBJCOPY) -O ihex -R .eeprom bin/quartz.o bin/quartz.hex

deps:
	mkdir -p vendor/libs
	mkdir -p vendor/build
	$(CC) -c vendor/tiny-midi-clock/midi_clock.c -mmcu=attiny2313 -o vendor/build/libtiny-midi-clock.o
	ar -cvq vendor/libs/libtiny-midi-clock.a vendor/build/libtiny-midi-clock.o

clean:
	rm -rf bin
	rm -rf vendor/build
	rm -rf vendor/libs

upload:
	sudo avrdude -p attiny2313 -c avrisp2 -U lfuse:w:0xFF:m -U hfuse:w:0xFF:m -U efuse:w:0xFF:m -U flash:w:bin/quartz.hex:i -b 2400 

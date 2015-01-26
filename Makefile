CC=avr-gcc
OBJCOPY=avr-objcopy

all: clean
	mkdir bin
	$(CC) src/geode.c -I./deps -mmcu=attiny2313 -o bin/geode.o
	$(OBJCOPY) -O ihex -R .eeprom bin/geode.o bin/geode.hex

clean:
	rm -rf bin

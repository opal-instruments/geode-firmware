CC=avr-gcc
SRC=$(wildcard src/*.c)
OBJCOPY=avr-objcopy
LIB_DIR=./vendor/libs/
LIBS=-static

all: clean fmt deps
	mkdir -p bin
	$(CC) src/quartz.c -L$(LIB_DIR) $(LIBS) -mmcu=attiny2313 -Os -o bin/quartz.o
	$(OBJCOPY) -O ihex -R .eeprom bin/quartz.o bin/quartz.hex

deps:
	mkdir -p vendor/libs
	mkdir -p vendor/build
	ar -cvq vendor/libs/libtiny-midi-clock.a 

clean:
	rm -rf bin
	rm -rf vendor/build
	rm -rf vendor/libs

fmt:
	astyle --style=google --align-pointer=type --align-reference=type $(SRC)

upload:
	sudo avrdude -p attiny2313 -c avrisp2 -U lfuse:w:0xFF:m -U hfuse:w:0xFF:m -U efuse:w:0xFF:m -U flash:w:bin/quartz.hex:i -b 2400 

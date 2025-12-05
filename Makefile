WARNINGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wstrict-prototypes -Wno-array-bounds

CFLAGS := -Os -DF_CPU=16000000UL -mmcu=atmega328p $(WARNINGS) -I . -I ./micro-ecc-master

all: projet.hex
	echo compilation terminee

upload: projet.hex
	avrdude -v -patmega328p -carduino -P/dev/ttyACM0 -b115200 -D -Uflash:w:$^

memoire.o: memoire.c memoire.h salt.h
	avr-gcc $(CFLAGS) -o $@ -c $<

alea.o: alea.c alea.h blake2s.h salt.h
	avr-gcc $(CFLAGS) -o $@ -c $<

blake2s.o: blake2s.c blake2s.h
	avr-gcc $(CFLAGS) -o $@ -c $<

consent.o: consent.c consent.h
	avr-gcc $(CFLAGS) -o $@ -c $<

projet.hex: projet.elf
	avr-objcopy -O ihex $^ $@

projet.elf: main.c consent.o memoire.o alea.o blake2s.o uECC.o signature.o ctap.o uart.o libring_buffer.a
	avr-gcc $(CFLAGS) -o $@ $^

uECC.o:
	avr-gcc --no-warnings -Os -DF_CPU=16000000UL -mmcu=atmega328p -I . -I ./micro-ecc-master -o $@ -c ./micro-ecc-master/uECC.c

signature.o: signature.c signature.h alea.h ./micro-ecc-master/uECC.h
	avr-gcc $(CFLAGS) -o $@ -c $<

uart.o: uart.c uart.h ring_buffer.h
	avr-gcc $(CFLAGS) -o $@ -c $<

ctap.o: ctap.c ctap.h memoire.h signature.h uart.h alea.h consent.h
	avr-gcc $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o *.elf *.hex *~ *#*

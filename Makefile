PREFIX=/home/laxis/avr/bin

PROJECT=avr-sensor-gateway
GCC_PART=atmega328p
AVRDUDE_PART=m328p

CFLAGS=-mmcu=$(GCC_PART) -Wall -O2 -DF_CPU=8000000

SOURCES=$(wildcard *.c)
INCLUDES=-I.

all: $(PROJECT).hex

$(PROJECT).out: $(SOURCES)
	$(PREFIX)/avr-gcc $(CFLAGS) $(INCLUDES) -o $@ $^
	$(PREFIX)/avr-objdump -d -S -l $@ >$(PROJECT).disasm

$(PROJECT).hex: $(PROJECT).out
	$(PREFIX)/avr-objcopy -O ihex $< $@
	$(PREFIX)/avr-size $<

flash: $(PROJECT).hex
	sudo $(PREFIX)/avrdude -p $(AVRDUDE_PART) -c usbasp -U flash:w:$<:i

# lfuse:0x62 = internal oscillator 1MHz
# lfuse:0xe2 = internal oscillator 8MHz
fuses:
	sudo $(PREFIX)/avrdude -p $(AVRDUDE_PART) -c usbasp -U lfuse:w:0xe2:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m

clean:
	@rm $(PROJECT).out $(PROJECT).hex $(PROJECT).disasm

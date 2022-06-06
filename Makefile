local:
	g++ -o bin/local.bin localsrc/main.cpp localsrc/SerialPort.cpp

embedded:
	avr-gcc -Os -mmcu=atmega328p -o bin/embedded.bin embeddedsrc/embedded.c
	avr-objcopy -O ihex -R .eeprom bin/embedded.bin bin/embedded.hex
	sudo avrdude -F -V -c arduino -p ATMEGA328P -P /dev/ttyACM0 -b 115200 -U flash:w:bin/embedded.hex
	rm bin/embedded.hex
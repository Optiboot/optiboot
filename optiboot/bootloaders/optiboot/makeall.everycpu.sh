#
# Makefile to make one image for each chip that optiboot supports.
# includes Arduino, Spence Konde ATtinyCore, Hans MCUDude cores, and
# some others.
# this is mainly used to sanity check modifications, especially those
# that are expected to cause "no binary changes" (you can diff the
# resulting .hex file with a reference implementation.)
#

make clean

#Attinycore
make attiny841at16
make attiny841at16ser1
make attiny441at16
make attiny441at16ser1
make attiny87at16
make attiny167at16
make attiny1634at16
make attiny1634at16ser1
make attiny828at16
make attiny88at16
make attiny48at16
make attiny85at16
make attiny45at16
make attiny84at16
make attiny44at16
make attiny861at16
make attiny461at16

# mega
make mega2560
make mega1280

# usb
make atmega8u2   LED=D5 LED_START_FLASHES=2 UART=1
make atmega16u2  LED=D5 LED_START_FLASHES=2 UART=1
make atmega32u2  LED=D5 LED_START_FLASHES=2 UART=1
make atmega16u4  LED=C7 LED_START_FLASHES=0 UART=1
make atmega32u4  LED=C7 LED_START_FLASHES=0 UART=1
make atmega32u6  LED=C6 LED_START_FLASHES=2 UART=1
make at90usb646  LED=C6 LED_START_FLASHES=2 UART=1  NODATE=1 BIGBOOT=1
make at90usb647  LED=C6 LED_START_FLASHES=2 UART=1  NODATE=1 BIGBOOT=1
make at90usb1286  LED=C6 LED_START_FLASHES=2 UART=1 NODATE=1 BIGBOOT=1
make at90usb1287  LED=C6 LED_START_FLASHES=2 UART=1 NODATE=1 BIGBOOT=1

#  buildable platforms of somewhat questionable support level
make lilypad
make pro8
make pro16
make pro20
make atmega328_pro8
make sanguino
make mega1280
make luminet
make diecimila
make bobuino
make wildfirev2
make atmega1284
make atmega32
make atmega168p

# Atmel development board targets
make xplained168pb
make xplained328p
make xplained328pb

# mcudude cores
make atmega8     LED=B5 LED_START_FLASHES=2 UART=0
make atmega16    LED=B0 LED_START_FLASHES=2 UART=0
make atmega32    LED=B0 LED_START_FLASHES=2 UART=0
make atmega64    LED=B5 LED_START_FLASHES=2 UART=0 BIGBOOT=1 NODATE=1
make atmega88    LED=B5 LED_START_FLASHES=2 UART=0
make atmega88p   LED=B5 LED_START_FLASHES=2 UART=0
make atmega88pb  LED=B5 LED_START_FLASHES=2 UART=0
make atmega128   LED=B5 LED_START_FLASHES=2 UART=0 BIGBOOT=1 NODATE=1
make atmega162   LED=B0 LED_START_FLASHES=0 UART=0
make atmega164a  LED=B0 LED_START_FLASHES=2 UART=0
make atmega164p  LED=B0 LED_START_FLASHES=2 UART=0
make atmega168   LED=B5 LED_START_FLASHES=2 UART=0
make atmega168p  LED=B5 LED_START_FLASHES=2 UART=0
make atmega168pb LED=B5 LED_START_FLASHES=2 UART=0
make atmega169   LED=B5 LED_START_FLASHES=2 UART=0
make atmega169p  LED=B5 LED_START_FLASHES=2 UART=0
make atmega324a  LED=B0 LED_START_FLASHES=2 UART=0
make atmega324p  LED=B0 LED_START_FLASHES=2 UART=0
make atmega324pa LED=B0 LED_START_FLASHES=2 UART=0
make atmega324pb LED=B0 LED_START_FLASHES=2 UART=0
make atmega328   LED=B5 LED_START_FLASHES=2 UART=0
make atmega328pb LED=B5 LED_START_FLASHES=2 UART=0
make atmega329   LED=B5 LED_START_FLASHES=2 UART=0
make atmega329p  LED=B5 LED_START_FLASHES=2 UART=0
make atmega640   LED=B7 LED_START_FLASHES=2 UART=0 BIGBOOT=1 NODATE=1
make atmega644p  LED=B0 LED_START_FLASHES=2 UART=0 BIGBOOT=1 NODATE=1
make atmega649   LED=B5 LED_START_FLASHES=2 UART=0 BIGBOOT=1 NODATE=1
make atmega649p  LED=B5 LED_START_FLASHES=2 UART=0 BIGBOOT=1 NODATE=1
make atmega1280  LED=B7 LED_START_FLASHES=2 UART=0 BIGBOOT=1 NODATE=1
make atmega1281  LED=B5 LED_START_FLASHES=2 UART=0 BIGBOOT=1 NODATE=1
make atmega1284  LED=B0 LED_START_FLASHES=2 UART=0 BIGBOOT=1 NODATE=1
make atmega1284p LED=B0 LED_START_FLASHES=2 UART=0 BIGBOOT=1 NODATE=1
make atmega2560  LED=B7 LED_START_FLASHES=2 UART=0 BIGBOOT=1 NODATE=1
make atmega2561  LED=B5 LED_START_FLASHES=2 UART=0 BIGBOOT=1 NODATE=1
make atmega3290  LED=B5 LED_START_FLASHES=2 UART=0
make atmega3290p LED=B5 LED_START_FLASHES=2 UART=0
make atmega6490  LED=B5 LED_START_FLASHES=2 UART=0 BIGBOOT=1 NODATE=1
make atmega6490p LED=B5 LED_START_FLASHES=2 UART=0 BIGBOOT=1 NODATE=1
make atmega8515  LED=B0 LED_START_FLASHES=2 UART=0
make atmega8535  LED=B0 LED_START_FLASHES=2 UART=0

# default generic platforms
# These need to be built AFTER the platforms, or they'll get renamed
make atmega8
make atmega168
make atmega328


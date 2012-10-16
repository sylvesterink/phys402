AVRPATH = ~/avr/avr8-gnu-toolchain-linux_x86/bin
CXX = $(AVRPATH)/avr-gcc
OBJCPY = $(AVRPATH)/avr-objcopy
LOADER = avrdude
CFLAGS = -Wall -mmcu=atxmega128a1 -Os#-g
LINKFLAGS = -mmcu=atxmega128a1
LIBRARIES = -lAVRX_Clocks -lAVRX_Serial -lm -lc
INCLOCS =
PROGRAMMER = avrispmkII
DEVICE = x128a1 
PORT = usb
SOURCE  = part1.c
OBJECTS = $(SOURCE:.c=.o)
EXECUTABLE = part1

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $(EXECUTABLE) $(OBJECTS) $(INCLOCS) $(LIBRARIES) $(LINKFLAGS)
	$(OBJCPY) -j .text -j .data -O ihex $(EXECUTABLE) $(EXECUTABLE).hex

.c.o:
	$(CXX) -c $(INCLOCS) $(CFLAGS) $< -o $@

load:
	$(LOADER) -c $(PROGRAMMER) -p $(DEVICE) -P $(PORT) -U flash:w:$(EXECUTABLE).hex

clean:
	rm -rf *.o $(EXECUTABLE) $(EXECUTABLE).hex

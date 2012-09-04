CXX = avr-gcc
OBJCPY = avr-objcopy
LOADER = avrdude
CFLAGS = -Wall -mmcu=atmega64 -Os #-g
LINKFLAGS = -mmcu=atmega64
LIBRARIES = #-lpthread -lrt -L$(WSU_LIB) -lris -ltpsapi -lipc
INCLOCS =  #-I/usr/avr/include#-I$(WSU_INC)
PROGRAMMER = avrispmkII
DEVICE = m64 
PORT = usb
SOURCE  = main.c
OBJECTS = $(SOURCE:.c=.o)
EXECUTABLE = ledblink

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $(EXECUTABLE) $(OBJECTS) $(INCLOCS) $(LIBRARIES) $(LINKFLAGS)
	$(OBJCPY) -j .text -j .data -O ihex $(EXECUTABLE) $(EXECUTABLE).hex

.c.o:
	$(CXX) -c $(INCLOCS) $(CFLAGS) $< -o $@

load:
	$(LOADER) -c $(PROGRAMMER) -p $(DEVICE) -P $(PORT) -U flash:w:$(EXECUTABLE).hex

clean:
	rm -rf *.o $(EXECUTABLE)

AVRPATH = ~/avr/avr8-gnu-toolchain-linux_x86/bin
CXX = $(AVRPATH)/avr-gcc
OBJCPY = $(AVRPATH)/avr-objcopy
LOADER = avrdude
CFLAGS = -Wall -mmcu=atxmega128a1 -Os#-g
LINKFLAGS = -mmcu=atxmega128a1
LIBRARIES = -lAVRX_Clocks#-L~/avr/avrfiles#-lpthread -lrt -L$(WSU_LIB) -lris -ltpsapi -lipc
INCLOCS =  #-I~/avr/avrfiles#-I/usr/avr/include#-I$(WSU_INC)
PROGRAMMER = avrispmkII
DEVICE = x128a1 
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
	rm -rf *.o $(EXECUTABLE) $(EXECUTABLE).hex

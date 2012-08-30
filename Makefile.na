#VERSION_FILE = .build_version
#BUILD_DATE = $(shell date +'%Y%m%d')
#BUILD_VER = $(shell cut -d. -f3 $(VERSION_FILE))
#MIN_VER = $(shell cut -d. -f2 $(VERSION_FILE))
#MAJ_VER = $(shell cut -d. -f1 $(VERSION_FILE))

#BASEDIR = /home/user/
#WSU_LIB = $(BASEDIR)trunk/something/lib
#WSU_INC = $(BASEDIR)trunk/something/include/

CXX = avr-gcc
CFLAGS = -Wall -Wold-style-cast#-g
#CFLAGS += -DBUILD_DATE=$(BUILD_DATE)\
		  #-DBUILD_NUM=$(BUILD_VER)\
		  #-DMIN_VERSION=$(MIN_VER)\
		  #-DMAJ_VERSION=$(MAJ_VER)\
		  #-DMINIWSU
LINKFLAGS = #-march=armv5te -mtune=arm926ej-s -mthumb-interwork -mthumb
LIBRARIES = #-lpthread -lrt -L$(WSU_LIB) -lris -ltpsapi -lipc
INCLOCS =   #-I$(WSU_INC)
SOURCE  = main.c
OBJECTS = $(SOURCE:.cpp=.o)
EXECUTABLE = a.out

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $(EXECUTABLE) $(OBJECTS) $(INCLOCS) $(LIBRARIES) $(LINKFLAGS)

.cpp.o:
	$(CXX) -c $(INCLOCS) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)

#newbuild:
	#@if ! test -f $(VERSION_FILE); then echo 0.0.0 > $(VERSION_FILE); fi
	#@echo $$(cut -d. -f1 $(VERSION_FILE)).$$(cut -d. -f2 $(VERSION_FILE)).$$(($$(cut -d. -f3 $(VERSION_FILE)) + 1)) > $(VERSION_FILE)
	#@touch main.cpp
	#@echo New Build $$(cat $(VERSION_FILE))

#newminor:
	#@if ! test -f $(VERSION_FILE); then echo 0.0.0 > $(VERSION_FILE); fi
	#@echo $$(cut -d. -f1 $(VERSION_FILE)).$$(($$(cut -d. -f2 $(VERSION_FILE)) + 1)).0 > $(VERSION_FILE)
	#@touch main.cpp
	#@echo New Minor Version $$(cat $(VERSION_FILE))

#newmajor:
	#@if ! test -f $(VERSION_FILE); then echo 0.0.0 > $(VERSION_FILE); fi
	#@echo $$(($$(cut -d. -f1 $(VERSION_FILE)) + 1)).0.0 > $(VERSION_FILE)
	#@touch main.cpp
	#@echo New Major Version $$(cat $(VERSION_FILE))

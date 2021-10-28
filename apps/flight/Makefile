OBJECTS = main.o \
          adc.o \
          pkts.o \
          gps.o \
          telem.o

EXEC = flight

DEPS = libmspuartlink:gcc libartibeus:gcc libads:gcc libio:gcc libfixed:gcc \
				libmspware:gcc libmsp:gcc libgnss:gcc liblsm9ds1:gcc

include tools/maker/Makefile

ifneq ($(FLIGHT_PRELOAD),)
override CFLAGS += -DFLIGHT_PRELOAD
endif



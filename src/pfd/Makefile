PREFIX ?= /usr/local
LIBRARY_PATH ?= lib
BINARY_PATH  ?= bin

INSTALL_INCLUDE_PATH = $(DESTDIR)$(PREFIX)/$(INCLUDE_PATH)
INSTALL_LIBRARY_PATH = $(DESTDIR)$(PREFIX)/$(LIBRARY_PATH)
INSTALL_BINARY_PATH  = $(DESTDIR)$(PREFIX)/$(BINARY_PATH)

INSTALL ?= cp -a

CFLAGS := $(CFLAGS) -I../common -I$(CARRIER_DIST_PATH)/include $(INSTALL_INCLUDE_PATH) -Wall  -Wstrict-prototypes -Wwrite-strings

LIBS = -L$(CARRIER_DIST_PATH)/lib -lelacommon -lelacarrier -lelasession -lconfuse -lpthread

OBJS = pfd.o config.o 

.PHONY: all clean install

all: elapfd

elapfd: $(OBJS)
	$(CC) -o $@ $(OBJS) -L$(INSTALL_LIBRARY_PATH) $(LIBS)

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

install: all
	mkdir -p $(INSTALL_BINARY_PATH)
	$(INSTALL) elapfd $(INSTALL_BINARY_PATH)

clean:
	rm -rf elapfd *.o

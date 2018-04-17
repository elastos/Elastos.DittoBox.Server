BINNAME = elaoc-agentd
PREFIX ?= /usr/local
LIBRARY_PATH ?= lib
BINARY_PATH  ?= bin
INCLUDE_PATH ?= include

INSTALL_INCLUDE_PATH = $(DESTDIR)$(PREFIX)/$(INCLUDE_PATH)
INSTALL_LIBRARY_PATH = $(DESTDIR)$(PREFIX)/$(LIBRARY_PATH)
INSTALL_BINARY_PATH  = $(DESTDIR)$(PREFIX)/$(BINARY_PATH)
CARRIER_DIST_PATH   := $(CARRIER_DIST_PATH)/$(HOST)-$(ARCH)/$(BUILD)

INSTALL ?= cp -a

CFLAGS := $(CFLAGS) -I../common -I$(CARRIER_DIST_PATH)/include -I$(INSTALL_INCLUDE_PATH) -Wall  -Wstrict-prototypes -Wwrite-strings

LIBS = -L$(CARRIER_DIST_PATH)/lib -lelacommon -lelacarrier -lelasession -lconfuse -lpthread

ifneq (,$(findstring $(HOST), Linux Raspbian))
        LIBS := $(LIBS) -lrt
endif

OBJS = pfd.o config.o 

.PHONY: all clean install

all: $(BINNAME)

$(BINNAME): $(OBJS)
	@echo $(DARRIER_DIST_PATH)
	$(CC) -o $@ $(OBJS) -L$(INSTALL_LIBRARY_PATH) $(LIBS)

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

install: all
	mkdir -p $(INSTALL_BINARY_PATH)
	$(INSTALL) $(BINNAME) $(INSTALL_BINARY_PATH)

clean:
	rm -rf $(BINNAME) *.o

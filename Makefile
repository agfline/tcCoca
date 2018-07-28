
export CC = gcc
export CFLAGS = -W -Wall -g -O3
SRC = lib/libTC.c tcCoca.c
BINDIR = ./bin


.PHONY: clean



all: $(BINDIR)/tcCoca

clean:
	rm -f $(BINDIR)/tcCoca


UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
linux32: $(BINDIR)/tcCoca-linux32
linux64: $(BINDIR)/tcCoca-linux64

win32: $(BINDIR)/tcCoca-win32.exe
win64: $(BINDIR)/tcCoca-win64.exe
endif

ifeq ($(UNAME_S),Darwin)
mac32: $(BINDIR)/tcCoca-mac32
mac64: $(BINDIR)/tcCoca-mac64
endif


$(BINDIR)/tcCoca-win32.exe: lib/libTC.c tcCoca.c
	i686-w64-mingw32-$(CC) $(SRC) -o $@ $(CFLAGS)

$(BINDIR)/tcCoca-win64.exe: lib/libTC.c tcCoca.c
	x86_64-w64-mingw32-$(CC) $(SRC) -o $@ $(CFLAGS)

$(BINDIR)/tcCoca-linux32: lib/libTC.c tcCoca.c
	$(CC) -o $@ $(SRC) $(CFLAGS) -lm -m32

$(BINDIR)/tcCoca-linux64: lib/libTC.c tcCoca.c
	$(CC) -o $@ $(SRC) $(CFLAGS) -lm -m64

$(BINDIR)/tcCoca-mac32: lib/libTC.c tcCoca.c
	$(CC) -o $@ $(SRC) $(CFLAGS) -lm -m32

$(BINDIR)/tcCoca-mac64: lib/libTC.c tcCoca.c
	$(CC) -o $@ $(SRC) $(CFLAGS) -lm -m64


$(BINDIR)/tcCoca: lib/libTC.c tcCoca.c
	$(CC) -o $@ $(SRC) $(CFLAGS) -lm

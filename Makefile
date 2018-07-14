
export CC = gcc
export CFLAGS = -W -Wall -g -O3

BINDIR = ./bin


.PHONY: clean



all: clean $(BINDIR)/tcCoca

clean:
	rm -f $(BINDIR)/tcCoca

$(BINDIR)/tcCoca: lib/libTC.c tcCoca.c
	$(CC) -o $@ $^ $(CFLAGS) -lm

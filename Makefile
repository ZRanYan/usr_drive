
include config.mk

all:
	./mk.sh

debug:
	./mk.sh debug

clean:
	./mk.sh clean

.PHONY: all debug clean


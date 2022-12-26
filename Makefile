# This builds the protocol-reader and message containers,
# and automatically runs the tests included in main.cpp.
#
# Implicit rules keep the Makefile a little smaller

CXXFLAGS = -Wall -Wextra -O2 -Isrc
.PHONY: run

run: bin/reader
	./bin/reader

clean:
	rm src/*.o
	rm bin/reader

bin/reader: src/main.o src/ProtocolMesg.o src/StreamDecoder.o
	$(CC) $^ -o $@

# This builds the protocol-reader and message containers,
# and automatically runs the tests included in main.cpp.
#
# Implicit rules keep the Makefile a little smaller

CXXFLAGS = -Wall -Wextra -O2 -Isrc -g
.PHONY: run clean debug default

default: bin/reader

run: bin/reader
	./bin/reader

debug: bin/reader
	gdb bin/reader

clean:
	rm src/*.o
	rm bin/reader

# All three of these files rely on both of the headers.
# (BJN: truth be told, compiling everything directly would be simpler,
# but this way make can run some dependency trees. If this were a larger project,
# setting up auto-dependencies would be worthwhile; this is faking that.)
src/main.o:          src/ProtocolMesg.hpp  src/StreamDecoder.hpp
src/StreamDecoder.o: src/ProtocolMesg.hpp  src/StreamDecoder.hpp

bin/reader: src/main.o src/StreamDecoder.o
	$(CXX) $^ -g -o $@


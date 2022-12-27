# Embedded Programming Exercise

This implements the stream decoder described in Brian's message.

## Building & tests

To build, run `make`. The build process relies only on `make` and `g++`,
and should work on any Linux machine with a reasonably modern copy of GCC.

To run the tests, execute `make run`. This will build the main executable then
run it.

## Structure
There are three main components to this demonstration.

### main.cpp
Main provides the entry point to the program, and implements a set of unit tests.
The first test shows broad functionality; the others focus on specific requirements
laid out in the example.

### StreamDecoder.cpp
StreamDecoder implements all the business logic to parse, checksum, store, and
retrieve messages. Messages are allocated on the heap, and the caller takes
responsibility to free the data after calling `popNextMessage()`.

### ProtocolMesg.hpp
ProtocolMesg and its child classes are very simple containers for message data.

## Test framework
I didn't want to set up gtest or a real unit test framework for a demonstration,
so these tests use C's native `assert` keyword and manual setup/teardown.
A single failed assert will immediately stop the program, so executing to the end
indicates every assertion passed.

## Notes
I've sprinkled some comments starting with `//BJN` throughout. These show
some of my thoughts about tradeoffs as I'm writing, and are generally things
I wouldn't comment on day-to-day.

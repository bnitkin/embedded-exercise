#ifndef STREAMDECODER_H
#define STREAMDECODER_H

#include <stdint.h>
#include <list>
#include <vector>
#include "ProtocolMesg.hpp"

class StreamDecoder {
  public:
     StreamDecoder() {
        this->reset();
     }

     // Clears any state in the StreamDecoder. Useful for recovery if
     // extra bytes arrive in the datastream.
     void reset();
     // Clear the partial-message buffer, but leave the recieved-message
     // list alone.
     void clearBuffer();


     // Parse many bytes of incoming data. Data may be segmented,
     // but messages must arrive in the same order as on the wire.
     // Splitting data in the middle of messages is fine, as is packing
     // multiple messages into one call to onDataFromChip.
     // Arguments:
     // uint8_t* data: A pointer to the data
     // int size: The number of bytes to read
     void onDataFromChip(uint8_t* data, int size) {
        // BJN: Placing this function in the header gives the compiler
        // flexibility to inline/optimize the loop for performance.
        // It also forces a copy of onDataFromChip into each generated
        // object file (duplication of generated code)
        for (int i=0; i<size; i++) {
           parseByte(data[i]);
        }
     }

     // Parse a single byte of incoming data. This performs the work
     // for onDataFromChip().
     // Argument:
     // uint8_t data: A single byte to parse
     void parseByte(uint8_t data);

     // Check whether a particular device has an unread message.
     // Argument: 
     // deviceId_e deviceId: The device to query for messages
     // Return:
     // `true` if there's a message ready, `false` otherwise.
     bool hasMessage(uint16_t deviceId);

     // Get the next message (in sequence order) from internal storage.
     // The actual class returned will depend on the deviceId - see
     // ProtocolMesg for details on fields for each message type.
     ProtocolMesg* popNextMessage(uint16_t deviceId);

   protected:
     // Messages is a list of recieved messages.
     // A list is nice since we never need to access elements by index.
     // Add a message to the end -> push O(1)
     // Find the most recent message for a device -> iterate O(n)
     // Remove that message -> pop O(1)
     std::list<ProtocolMesg*> messages;

     // Buffer of bytes that aren't yet processed into a message.
     std::vector<uint8_t> buffer;
     uint16_t expectedBytes;

     // Convenience function to get the number of bytes stored for the
     // message currently being recieved.
     uint16_t recievedBytes() {
        return this->buffer.size();
     }

     // Returns true if newSequence is lower than savedSequence, after unwrapping logic.
     bool betterSequenceNumber(uint8_t savedSequence, uint8_t newSequence);
};

#endif

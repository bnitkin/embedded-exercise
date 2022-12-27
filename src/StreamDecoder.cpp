#include "StreamDecoder.hpp"

// Clears any state in the StreamDecoder. Useful for recovery if
// extra bytes arrive in the datastream.
void StreamDecoder::reset() {
   // Erase both the recieved-message and processing buffers.
   this->messages.clear();
   this->buffer.clear();
   // Size of a packet with no payload.
   this->expectedBytes = 2 + 1 + 1 + 1 + 0 + 1;
}

// Parse a single byte of incoming data. This performs the work
// for onDataFromChip().
void StreamDecoder::parseByte(uint8_t data) {
   // Add new data to the buffer
   this->buffer.push_back(data);

   if (recievedBytes() == ProtocolMesg::MSG_TYPE+1) {
      // Compute the payload size and add to expected size
      // (Need to add one to compensate for zero-based indexing)
      switch (this->buffer[ProtocolMesg::DEVICE_TYPE]) {
         case ProtocolMesg::BLIP:
            // Payload size is unknown, but at least 1B.
            expectedBytes += 1;
            break;
         case ProtocolMesg::WIDGET:
            // Payload is always 6B.
            expectedBytes += 6;
            break;
         case ProtocolMesg::LATCH:
            // LATCH has a 1B payload with STATUS; 0 otherwise.
            if (this->buffer[ProtocolMesg::MSG_TYPE] == LatchMesg::STATUS) {
               expectedBytes += 1;
            }
            break;
         default:
            // BJN: This protocol doesn't allow for graceful recovery since it's
            // impossible to find breaks between messages. If there's a message
            // we can't ID, we can't find the payload size. Without that we can't
            // find the next message.
            printf("ERROR: Unknown device ID\n");
            return;
      }
   }

   if (this->buffer[ProtocolMesg::DEVICE_TYPE] == ProtocolMesg::BLIP
       && recievedBytes() == ProtocolMesg::PAYLOAD+1) {
      // Special case for BLIP message - grab the string size from
      // the first byte of payload.
      expectedBytes += this->buffer[ProtocolMesg::PAYLOAD];

   }

   if (recievedBytes() == this->expectedBytes) {
      // Verify checksum and close out
      uint8_t sum = 0;
      uint8_t checksum = this->buffer.back();
      this->buffer.pop_back();
      for (auto it : this->buffer) {
         printf("%d ", it);
         sum += it;
      }

      if (sum == checksum) {
          // Compute common fields
          uint16_t id      = this->buffer[ProtocolMesg::DEVICE_ID_1] << 8
                           | this->buffer[ProtocolMesg::DEVICE_ID_1];
          ProtocolMesg::deviceType_e devType  =
              static_cast<ProtocolMesg::deviceType_e>(
                  this->buffer[ProtocolMesg::DEVICE_TYPE]);
          uint8_t sequence = this->buffer[ProtocolMesg::SEQUENCE];
          uint8_t msgType  = this->buffer[ProtocolMesg::MSG_TYPE];
          if (devType == ProtocolMesg::BLIP) {
              this->messages.emplace_back(
                  BlipMesg(id, devType, sequence, msgType, "Hi there"));

          } else if (devType == ProtocolMesg::WIDGET) {
              uint16_t serial = this->buffer[WidgetMesg::SERIAL_1] << 8
                              | this->buffer[WidgetMesg::SERIAL_2];
              uint8_t batch   = this->buffer[WidgetMesg::BATCH];
              uint8_t version = this->buffer[WidgetMesg::VERSION_MAJOR] << 16
                              | this->buffer[WidgetMesg::VERSION_MINOR] << 8
                              | this->buffer[WidgetMesg::VERSION_PATCH];
              this->messages.emplace_back(
                  WidgetMesg(id, devType, sequence, msgType,
                             serial, batch, version));

          } else if (devType == ProtocolMesg::LATCH) {
              bool state = false;
              if (false) {}
              this->messages.emplace_back(
                  LatchMesg(id, devType, sequence, msgType, state));

          } else {
              printf("ERROR: Unknown device ID\n");
          }
      } else {
          printf("Bad checksum!\n");
      }
   }
}

// Check whether a particular device has an unread message.
bool StreamDecoder::hasMessage(uint16_t deviceId) {
   return true;
}

// Get the next message (in sequence order) from internal storage.
ProtocolMesg StreamDecoder::popNextMessage(uint16_t deviceId) {
   return ProtocolMesg(1, ProtocolMesg::WIDGET, 3, 4);
}

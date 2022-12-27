#include "StreamDecoder.hpp"

// Clears any state in the StreamDecoder. Useful for recovery if
// extra bytes arrive in the datastream.
void StreamDecoder::reset() {
    // Erase both the recieved-message and processing buffers.
    for (auto it : this->messages) {
        delete it;
    }
    this->messages.clear();
    this->clearBuffer();
}

// Clear the partial-message buffer, but leave the recieved-message
// list alone.
void StreamDecoder::clearBuffer() {
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
                // (In reality, you'd probably wait for a quiet time on the line and
                // reset all the buffers. In unit-test land, that sort of heuristic
                // doesn't make much sense.)
                printf("FATAL: Unknown device ID\n");
                return;
        }
    }

    if (this->buffer[ProtocolMesg::DEVICE_TYPE] == ProtocolMesg::BLIP
        && recievedBytes() == BlipMesg::SIZE+1) {
        // Special case for BLIP message - grab the string size from
        // the first byte of payload.
        expectedBytes += this->buffer[BlipMesg::SIZE];
    }

    if (recievedBytes() == this->expectedBytes) {
        // Verify checksum and close out
        uint8_t sum = 0;
        uint8_t checksum = this->buffer.back();
        this->buffer.pop_back();
        printf("INFO: Processing");
        for (auto it : this->buffer) {
            printf(" %02x", it);
            sum += it;
        }

        if (sum == checksum) {
            printf("\nINFO:     checksum OK");
            // Compute common fields
            uint16_t id      = this->buffer[ProtocolMesg::DEVICE_ID_1] << 8
                | this->buffer[ProtocolMesg::DEVICE_ID_2];
            ProtocolMesg::deviceType_e devType  =
                static_cast<ProtocolMesg::deviceType_e>(
                                                        this->buffer[ProtocolMesg::DEVICE_TYPE]);
            uint8_t sequence = this->buffer[ProtocolMesg::SEQUENCE];
            uint8_t msgType  = this->buffer[ProtocolMesg::MSG_TYPE];
            if (devType == ProtocolMesg::BLIP) {
                BlipMesg* blip = new BlipMesg(id, devType, sequence, msgType,
                    std::string(reinterpret_cast<const char*>(&this->buffer[BlipMesg::STRING]),
                    this->buffer[BlipMesg::SIZE]));
                    this->messages.push_back(blip);

                // BJN: This displays the actual payload, but the string could
                // have escape characters in it, and that'll mess with a terminal.
                //printf(": Blip message %s", blip->payload.c_str());
                printf(": Blip message %ldB", blip->payload.size());

            } else if (devType == ProtocolMesg::WIDGET) {
                uint16_t serial = this->buffer[WidgetMesg::SERIAL_1] << 8
                    | this->buffer[WidgetMesg::SERIAL_2];
                uint8_t batch   = this->buffer[WidgetMesg::BATCH];
                uint32_t version = this->buffer[WidgetMesg::VERSION_MAJOR] << 16
                    | this->buffer[WidgetMesg::VERSION_MINOR] << 8
                    | this->buffer[WidgetMesg::VERSION_PATCH];
                printf(": Widget message %04X batch %02X ver %06X",
                       serial, batch, version);
                    WidgetMesg* widget = new WidgetMesg(id, devType, sequence, msgType,
                               serial, batch, version);
                    this->messages.push_back(widget);

            } else if (devType == ProtocolMesg::LATCH) {
                bool open = false;
                if (this->buffer[ProtocolMesg::MSG_TYPE] == LatchMesg::STATUS) {
                    open = this->buffer[LatchMesg::STATE];
                } else if (this->buffer[ProtocolMesg::MSG_TYPE] == LatchMesg::OPEN) {
                    open = true;
                } else if (this->buffer[ProtocolMesg::MSG_TYPE] == LatchMesg::CLOSE) {
                    open = false;
                }
                printf(": Latch message %s", open ? "open" : "closed");
                LatchMesg* latch = new LatchMesg(id, devType, sequence, msgType, open);
                this->messages.push_back(latch);

            } else {
                // BJN: See other note about device ID.
                printf("FATAL: Unknown device ID");
            }
        } else {
            printf("\nWARNING: checksum BAD: Expected %02x; found %02x!", sum, checksum);
        }
        // Now that the message is this->messages, clear the buffer.
        printf("\n");
        this->clearBuffer();
    }
}

// Check whether a particular device has an unread message.
bool StreamDecoder::hasMessage(uint16_t deviceId) {
    for (auto it : this->messages) {
        if (it->deviceId == deviceId) {
            return true;
        }
    }
    return false;
}

// Get the next message (in sequence order) from internal storage.
ProtocolMesg* StreamDecoder::popNextMessage(uint16_t deviceId) {
    // Holding on to the iterator (rather than the unwrapped ProtocolMesg*
    // allows for deletion after iteration). messages.end() is always invalid,
    // so it's a good initial value.
    std::list<ProtocolMesg*>::iterator bestMessage = messages.end();
    for (std::list<ProtocolMesg*>::iterator it=messages.begin();
         it != messages.end(); it++) {
        if ((*it)->deviceId == deviceId) {
            // The double-dereferences are pretty ugly, though.
            if (bestMessage == messages.end() ||
                this->betterSequenceNumber((*bestMessage)->sequence, (*it)->sequence)) {
                bestMessage = it;
            }
        }
    }
    // I'm not certain if the bestMessage iterator is destroyed by
    // erasing its value from the list. Grab a pointer to the message first.
    ProtocolMesg* retVal = *bestMessage;
    if (bestMessage == messages.end()) {
        // If this were in a larger application, there'd be clever ways to handle
        // errors - logging, a watchdog reset, or something. For PC use I'm using
        // printf to indicate that something happened, and not trying to handle it.
        printf("ERROR: No message found.\n");
    } else {
        printf("INFO: Found message for %04x with sequence %02x.\n", deviceId, retVal->sequence);
        this->messages.erase(bestMessage);
    }
    return retVal;
}


// Returns true if newSequence is lower than savedSequence, after unwrapping logic.
bool StreamDecoder::betterSequenceNumber(uint8_t savedSequence, uint8_t newSequence) {
    // This handles wraparound - in the special case where the best sequence number
    // is low (1..50) but there exist high numbers (200..255), the high numbers are
    // older, due to wraparound. (This will fail if more than 50 messages are left
    // in the queue, or if sequence numbers jump for some reason.)
    //
    // BJN: Magic numbers aren't my favorite, but I think this is more readable
    // without naming the numbers.
    // (savedSequence < WRAPAROUND_FACTOR && newSequence > 0xFF-WRAPAROUND_FACTOR) {
    const uint8_t WRAPAROUND_LOW  = 50;
    const uint8_t WRAPAROUND_HIGH = 200;

    if (savedSequence > WRAPAROUND_HIGH && newSequence < WRAPAROUND_LOW) {
        // If saved is near wrapping and new is in the low region,
        // keep the saved one.
        return false;
    }
    if (savedSequence < WRAPAROUND_LOW && newSequence > WRAPAROUND_HIGH) {
        // If saved is in the low region and we find a number near wraparound,
        // pick that.
        return true;
    }

    // Otherwise - just pick the lowest number.
    return savedSequence > newSequence;
}

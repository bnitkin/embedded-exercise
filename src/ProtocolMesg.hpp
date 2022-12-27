#ifndef PROTOCOLMESG_H
#define PROTOCOLMESG_H

#include <string>

class ProtocolMesg {
    public:
        // The deviceType, deviceId, and index enums provide 
        // some helpful constants for working with messages.
        typedef enum {
            BLIP   = 0x05,
            WIDGET = 0x0F,
            LATCH  = 0x1F,
        } deviceType_e;

        typedef enum {
            DEVICE_ID_1 = 0,
            DEVICE_ID_2 = 1,
            DEVICE_TYPE = 2,
            SEQUENCE    = 3,
            MSG_TYPE    = 4,
            PAYLOAD     = 5
        } messageIndex_e;

        uint16_t deviceId;
        deviceType_e deviceType;
        uint8_t sequence;
        uint8_t messageType;

        ProtocolMesg(uint16_t id, deviceType_e devType, uint8_t sequence, uint8_t msgType) {
            this->deviceId    = id;
            this->deviceType  = devType;
            this->sequence    = sequence;
            this->messageType = msgType;
        }
};

// The below classes all specialize ProtocolMesg to enable protocol-agnostic fields.
//
// TODO: Add getters to prevent callers from changing fields without permission.
class BlipMesg: public ProtocolMesg {
    public:
        typedef enum {
            HELLO = 0x01,
        } messageType_e;

        typedef enum {
            SIZE = 5,
            STRING = 6,
        } payloadIndex_e;

        std::string payload;

        BlipMesg(uint16_t id, deviceType_e devType,
                 uint8_t sequence, uint8_t msgType, std::string payload) :
          ProtocolMesg(id, devType, sequence, msgType) {
            // This is a deep copy
            this->payload = payload;
        }
};

class WidgetMesg: public ProtocolMesg {
    public:
        typedef enum {
            VERSION_INFO = 0x01,
        } messageType_e;

        typedef enum {
            SERIAL_1 = 5,
            SERIAL_2 = 6,
            BATCH = 7,
            VERSION_MAJOR = 8,
            VERSION_MINOR = 9,
            VERSION_PATCH = 10,
        } payloadIndex_e;

        uint16_t serial;
        uint8_t batch;
        // Version is three bytes, but using a uint32 feels better than storing them
        // as separate elements.
        uint32_t version;

        WidgetMesg(uint16_t id, deviceType_e devType,
                   uint8_t sequence, uint8_t msgType,
                   uint16_t serial, uint8_t batch, uint32_t version) :
          ProtocolMesg(id, devType, sequence, msgType) {
            this->serial = serial;
            this->batch = batch;
            this->version = version;
        }
};

class LatchMesg: public ProtocolMesg {
    public:
        typedef enum {
            STATUS = 0x01,
            OPEN   = 0x02,
            CLOSED = 0x03,
        } messageType_e;

        // Returns the message-indicated state of the latch
        bool state;

        LatchMesg(uint16_t id, deviceType_e devType,
                   uint8_t sequence, uint8_t msgType,
                   bool state) :
          ProtocolMesg(id, devType, sequence, msgType) {
            this->state = state;
        }
};

#endif

#ifndef PROTOCOLMESG_H
#define PROTOCOLMESG_H

#include <string>

typedef enum {
   BLIP   = 0x05,
   WIDGET = 0x0F,
   LATCH  = 0x1F,
} deviceType_e;

class ProtocolMesg {
   public:
      const uint16_t deviceId;
      const deviceType_e deviceType;
      const uint8_t sequence;
      const uint8_t messageType;
};

// The below classes all specialize ProtocolMesg to enable protocol-agnostic fields.
// They all use const fields - this allows references to the raw messages to be passed
// around without letting external callers modify the message. The functionality is
// identical to non-const data that's only accessable through getters, but with less
// syntatical overhead.

class BlipMesg: public ProtocolMesg {
   public:
      typedef enum {
         HELLO = 0x01,
      } messageType_e;

      const std::string payload = "Hello, world!";
};

class WidgetMesg: public ProtocolMesg {
   public:
      typedef enum {
         VERSION_INFO = 0x01,
      } messageType_e;

      const uint16_t serial = 0xBEEF;
      const uint8_t batch = 0xFE;
      // Version is three bytes, but using a uint32 feels better than storing them
      // as separate elements.
      const uint32_t version = 0x01 << 16 | 0x05 << 8 | 0x93;
};

class LatchMesg: public ProtocolMesg {
   public:
      typedef enum {
         STATUS = 0x01,
         OPEN   = 0x02,
         CLOSED = 0x03,
      } messageType_e;

      // Returns the message-indicated state of the latch
      const bool state = true;
};

#endif

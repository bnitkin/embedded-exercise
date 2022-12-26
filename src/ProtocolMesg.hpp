#ifndef PROTOCOLMESG_H
#define PROTOCOLMESG_H
typedef enum {
   BLIP   = 0x05,
   WIDGET = 0x0F,
   LATCH  = 0x1F,
} deviceType_e;

class ProtocolMesgFactory {

};

class ProtocolMesg {
   public:
      ProtocolMessage();
      deviceType_e getDeviceType() {
         return BLIP;
      }

   protected:
      // Header overhead is ID:2 + Type:1 + Sequence:1 + Type:1 + Checksum:1 = 6B
      // Max body is 255+1B of string in Blip message.
      vector<uint8_t> data;
};

// The below classes all specialize ProtocolMesg to enable protocol-agnostic getters.

// BJN: All the behavior uses the internally-stored copy of the message and generate
// protocol-abstracted data on the fly. It could be more performant to preprocess the
// message into fields and only save the data from the messages. (But that depends
// on getters being queried many times per message; otherwise it's break-even)
class BlipMesg: public ProtocolMesg {
   public:
      typedef enum {
         HELLO = 0x01,
      } messageType_e;

      // Return a pointer to the string data.
      // BJN: const to keep the end user from modifying the underlying message
      const char* getPayload() {
         return "Hello, world!";
      }

      // Returns the string size from the message.
      uint8_t getSize() {
         return 5;
      }
};

class WidgetMesg: public ProtocolMesg {
   public:
      typedef enum {
         VERSION_INFO = 0x01,
      } messageType_e;

      // Returns the serial number from the message
      // BJN: I like inlining trivial functions like this by putting them in
      // the header, and reserving the .cpp file for complex logic.
      // That's obviously a matter of taste.
      uint16_t getSerial() {
         return 0xBEEF;
      }
      // Returns the batch number from the message
      uint8_t getBatch() {
         return 0xFE;
      }
      // Returns the version number, as three packed bytes, from the message
      // BJN: For human purposes, you could use a snprintf type thing
      // to format the numbers into a dot-separated string.
      uint32_t getVersion() {
         return 0x01 << 16 | 0x05 << 8 | 0x93;
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
      bool getState() {
         return true;
      }
};

#endif

#include <stdio.h>
#include "StreamDecoder.hpp"
#include "ProtocolMesg.hpp"

int main() {
   printf("Hello, world!\n");
   StreamDecoder decoder;
   //                ID    ID2   dev   seq   type  load....
   uint8_t data[] = {0x12, 0x24, 0x0F, 0x99, 0x01, 0xDE, 0xAD, 0x0F, 0x01, 0x01, 0x04, 0x7f};
   decoder.onDataFromChip(data, sizeof(data));
}

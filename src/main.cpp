#include <stdio.h>
#include <assert.h>
#include "StreamDecoder.hpp"
#include "ProtocolMesg.hpp"

void test_banner(int test_num, const char* description) {
    printf("\n\n============================================================\n");
    printf("====                   TEST % 2d                          ====\n", test_num);
    printf("==== %-51s====\n", description);
    printf("============================================================\n");
}

void test_1() {
    // Test 1: Feed in one of each message type.
    // This first test verifies that the reader's working properly. This confirms
    // that message size calculations & checksums work; it doesn't verify any fields.
    //
    // Formats are pulled from section 1 of the assignment.
    //
    // Checksums were sanity-checked with a Python one-liner to strip & sum the messages
    // hex(sum([int(x.strip(','), 16) for x in '0x03, 0x04, 0x05, 0x00, 0x01, 0x00'.split()]) & 0xff)

    test_banner(1, "Protocol Parsing");

    StreamDecoder decoder;
    // Fields are:      ID    ID2   dev   seq   type  payload........ checksum
    // Widget (device 0x1F)
    uint8_t data_1[] = {0x12, 0x24, 0x0F, 0x99, 0x01, 0xDE, 0xAD, 0x0F, 0x01, 0x01, 0x04, 0x7f};
    decoder.onDataFromChip(data_1, sizeof(data_1));

    // Fields are:      ID    ID2   dev   seq   type  payload........ checksum
    // Latch (device 0x1F) type 1
    uint8_t data_2[] = {0x01, 0x02, 0x1F, 0x00, 0x01, 0x01, 0x24};
    decoder.onDataFromChip(data_2, sizeof(data_2));
    // Latch (device 0x1F) type 2
    uint8_t data_3[] = {0x01, 0x02, 0x1F, 0x00, 0x02, 0x24};
    decoder.onDataFromChip(data_3, sizeof(data_3));
    // Latch (device 0x1F) type 3
    uint8_t data_4[] = {0x01, 0x02, 0x1F, 0x00, 0x03, 0x25};
    decoder.onDataFromChip(data_4, sizeof(data_4));

    // Fields are:      ID    ID2   dev   seq   type  payload........ checksum
    // Blip (device 0x05) empty string
    uint8_t data_5[] = {0x03, 0x04, 0x05, 0x00, 0x01, 0x00, 0x0d};
    decoder.onDataFromChip(data_5, sizeof(data_5));
    // Blip (device 0x05) short string
    uint8_t data_6[] = {0x01, 0x02, 0x05, 0x00, 0x01, 0x0c, 'h', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', 0x9d};
    decoder.onDataFromChip(data_6, sizeof(data_6));
    // Blip (device 0x05) short string, null-terminated
    uint8_t data_7[] = {0x01, 0x02, 0x05, 0x00, 0x01, 0x0d, 'h', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '\0', 0x9e};
    decoder.onDataFromChip(data_7, sizeof(data_7));
    // Blip (device 0x05) longest string
    uint8_t data_8[] = {0x01, 0x02, 0x05, 0x00, 0x01, 0xff,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, // 255, not 256 bytes.
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
        0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
        0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
        0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
        0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
        0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
        0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
        0x88};
    decoder.onDataFromChip(data_8, sizeof(data_8));
}

void test_2() {
    // Test 2: Split messages in new and exciting ways
    // This test takes a few basic messages and feeds them into onDataFromChip
    // in oddball ways. It demonstrates that multiple messages can be processed at once,
    // and that messages can be split across calls.
    // (Per parenthetical in section 2.0 of the assignment)
    test_banner(2, "Message merging/splitting");

    StreamDecoder decoder;
    // First set - merge data_1 - data_4 from test 1 into a single push
    uint8_t data_1[] = {0x12, 0x24, 0x0F, 0x99, 0x01, 0xDE, 0xAD, 0x0F, 0x01, 0x01, 0x04, 0x7f,
                        0x01, 0x02, 0x1F, 0x00, 0x01, 0x01, 0x24,
                        0x01, 0x02, 0x1F, 0x00, 0x02, 0x24,
                        0x01, 0x02, 0x1F, 0x00, 0x03, 0x25};
    decoder.onDataFromChip(data_1, sizeof(data_1));
    assert (decoder.hasMessage(0x1224));
    assert (decoder.hasMessage(0x0102));

    // Second set - take data_1 above and split it into bits.
    uint8_t data_2[] = {0x44, 0x24};
    uint8_t data_3[] = {0x0F, 0x99};
    uint8_t data_4[] = {0x01, 0xDE, 0xAD, 0x0F, 0x01, 0x01, 0x04, 0xB1, 0x01, 0x02, 0x1F, 0x00, 0x01, 0x01};
    uint8_t data_5[] = {0x74};
    uint8_t data_6[] = {0x01, 0x52, 0x1F};
    uint8_t data_7[] = {0x00, 0x02};
    uint8_t data_8[] = {0x24, 0x01, 0x52, 0x1F, 0x00, 0x03, 0x75};
    decoder.onDataFromChip(data_2, sizeof(data_2));
    decoder.onDataFromChip(data_3, sizeof(data_3));
    decoder.onDataFromChip(data_4, sizeof(data_4));
    decoder.onDataFromChip(data_5, sizeof(data_5));
    decoder.onDataFromChip(data_6, sizeof(data_6));
    decoder.onDataFromChip(data_7, sizeof(data_7));
    decoder.onDataFromChip(data_8, sizeof(data_8));
    assert (decoder.hasMessage(0x4424));
    assert (decoder.hasMessage(0x0152));
}

void test_3() {
    // Test 2: Show that messages with bad checksums are discarded
    // (Per penultimate paragraph in section 2.0)
    test_banner(3, "Bad checksums");

    StreamDecoder decoder;
    // Reusing data set from test 2
    uint8_t data_1[] = {0x12, 0x24, 0x0F, 0x99, 0x01, 0xDE, 0xAD, 0x0F, 0x01, 0x01, 0x04, 0x70,
                        0x01, 0x02, 0x1F, 0x00, 0x01, 0x01, 0x25,
                        0x01, 0x02, 0x1F, 0x00, 0x02, 0x20,
                        0x01, 0x02, 0x1F, 0x00, 0x03, 0x2A};
    decoder.onDataFromChip(data_1, sizeof(data_1));
    assert (decoder.hasMessage(0x1224) == false);
    assert (decoder.hasMessage(0x0102) == false);
}

int main() {
    printf("Hello, world!\n");
    test_1();
    test_2();
    test_3();
}

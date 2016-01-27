#include "hexdump.h"

#include <cstdio>
#include <cctype>

static void HexDumpLine(int address, uint8_t* data, int size);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void teletraffic::HexDump(int address, uint8_t* data, int size) {

      const int kWidth = 16;

      int i;
      int numlines = size / kWidth;
      int restchrs = size % kWidth;

      for (i = 0; i < numlines; i++) {
            HexDumpLine(address + i*kWidth, data + i*kWidth, kWidth);
            printf("\n");
      }

      if (restchrs > 0) {
            HexDumpLine(address + i*kWidth, data + i*kWidth, size % kWidth);
            printf("\n");
      }

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void HexDumpLine(int address, uint8_t* data, int size) {
      
      // Dirección
      printf("%08x ", address);

      // Bytes en forma hexadecimal
      for (int i = 0; i < size; i++) {
            printf("%02x ", data[i]);
      }

      // Representación ASCII
      printf("|");

      for (int i = 0; i < size; i++) {

            if (isprint(data[i])) {
                  printf("%c", data[i]);
            } else {
                  printf(".");
            }

      }

      printf("|");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

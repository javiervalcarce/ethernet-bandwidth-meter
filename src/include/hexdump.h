// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_HEXDUMP_RX_
#define TELETRAFFIC_HEXDUMP_RX_

#include <stdint.h>

namespace teletraffic {

void HexDump(int address, uint8_t* data, int size);
//void HexDumpLine(int address, uint8_t* data, int size);

}

#endif // TELETRAFFIC_HEXDUMP_RX_

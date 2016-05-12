// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include "utils.h"
#include <cstdio>
#include <cstring>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void teletraffic::MacAddressAsciiz2Binary(const char*   str,    uint8_t mac[6]) {
      unsigned int intmac[6];
      sscanf(str, "%x:%x:%x:%x:%x:%x", &intmac[0], &intmac[1], &intmac[2], &intmac[3], &intmac[4], &intmac[5]);

      int i;
      for (i = 0; i < 6; i++) {
            mac[i] = (uint8_t) intmac[i];
      }
      
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void teletraffic::MacAddressBinary2Asciiz(const uint8_t mac[6], char*   str) {
      snprintf(str, 18, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);     
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_UTILS_H_
#define TELETRAFFIC_UTILS_H_

#include <stdint.h>

namespace teletraffic {


      /**
       * Utilidades para convertir entre 2 posibles formatos de una dirección MAC.
       *
       * - formato cadena hexadecimal: 17 bytes + 1 byte para '\0', por ejemplo: "aa:bb:cc:dd:ee:ff"
       * - formato binario: 6 bytes 
       *
       * Esta función convierte de formato binario a cadena.
       */     
      void MacAddressAsciiz2Binary(const char*   str,    uint8_t mac[6]);

      /**
       * Utilidades para convertir entre 2 posibles formatos de una dirección MAC.
       *
       * - formato cadena hexadecimal: 17 bytes + 1 byte para '\0', por ejemplo: "aa:bb:cc:dd:ee:ff"
       * - formato binario: 6 bytes 
       *
       * Esta función convierte de formato binario a cadena.
       */
      void MacAddressBinary2Asciiz(const uint8_t mac[6], char*   str);

}

#endif // TELETRAFFIC_UTILS_H_


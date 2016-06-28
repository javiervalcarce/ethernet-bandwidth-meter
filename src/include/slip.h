// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_SLIP_H_
#define TELETRAFFIC_SLIP_H_

#include <stdint.h>

namespace teletraffic {
      
      class Slip { 
      public:
            Slip(int fd);
            ~Slip();

            int Send(uint8_t* data, int len);
            int Recv(uint8_t* data, int len);

      private:
            int fd_;
            int send_char(char c);
            int recv_char();
      };
}

#endif  // TELETRAFFIC_SLIP_H_


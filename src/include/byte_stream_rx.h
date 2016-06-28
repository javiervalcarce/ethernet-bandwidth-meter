// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_BYTE_STREAM_RX_H_
#define TELETRAFFIC_BYTE_STREAM_RX_H_

#include <stdint.h>
#include "service_thread.h"
#include "slip.h"

namespace teletraffic {

      class ByteStreamRx : public ServiceThread {
      public:
            ByteStreamRx(int fd);
            ~ByteStreamRx();
      private:
            int ServiceInit(); 
            int ServiceIteration();
      };
}

#endif   // TELETRAFFIC_BYTE_STREAM_RX_H_


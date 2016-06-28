// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_BYTE_STREAM_TX_H_
#define TELETRAFFIC_BYTE_STREAM_TX_H_

#include <stdint.h>
#include "service_thread.h"
#include "slip.h"

namespace teletraffic {

      class ByteStreamTx : public ServiceThread {
      public:
            ByteStreamTx(int fd);
            ~ByteStreamTx();
      private:
            int ServiceInit();
            int ServiceIteration();
      };
}

#endif   // TELETRAFFIC_BYTE_STREAM_TX_H_


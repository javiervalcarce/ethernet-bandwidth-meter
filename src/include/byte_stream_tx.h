// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_BYTE_STREAM_TX_H_
#define TELETRAFFIC_BYTE_STREAM_TX_H_

#include <termios.h>
#include <fcntl.h>

#include <stdint.h>
#include <string>

#include "service_thread.h"
#include "slip.h"


namespace teletraffic {

      /**
       * 
       */
      class ByteStreamTx : public ServiceThread {
      public:
            
            ByteStreamTx(std::string hostname, int port);
            ~ByteStreamTx();
            
            uint64_t SentByteCount();
            
      private:
            int fd_;
            uint64_t count_;
            Slip* framer_;
            struct termios serial_conf_;

            std::string hostname_;
            int port_;

            // Base class overrides
            int ServiceInitialize();
            int ServiceStart();
            int ServiceStop();
            int ServiceIteration();
            int ServiceFinalize();
      };

}

#endif   // TELETRAFFIC_BYTE_STREAM_TX_H_


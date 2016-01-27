// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_TELETRAFFIC_TX_
#define TELETRAFFIC_TELETRAFFIC_TX_

#include <pthread.h>
#include <pcap/pcap.h>
#include <string>

#include "stopwatch.h"

namespace teletraffic {

      /**
       * Flow source properties
       */
      struct FlowSource {
            std::string source_interface;
            int speed_bytes_per_second;
            int protocol_id;
            int packet_size;
            char dest_mac[6];
      };
      
      
class TeletrafficTx {
public:

      
      /**
       * Constructor.
       * 
       * @param interface Network interface used to transmit packets, every packet will [have protocol_id] in their
       * protocol field.
       *
       * @param protocol_id Protocol id for each packet.
       */
      TeletrafficTx(std::string interface, uint16_t protocol_id);
      
      /**
       * Destructor.
       */
      ~TeletrafficTx();

      /**
       * Initialization.
       */
      int Init();
      
      
private:

      std::string interface_;
      uint16_t protocol_id_;

      pcap_t* txdev_;
      
      char* errbuf_;
      char* pkt_sent_;
      Stopwatch watch_;

      pthread_t thread_;
      pthread_attr_t thread_attr_;
      bool initialized_;
      bool thread_exit_;

      static
      void* ThreadFn(void* obj);
      void* ThreadFn();
};

}

#endif // TELETRAFFIC_TELETRAFFIC_TX_


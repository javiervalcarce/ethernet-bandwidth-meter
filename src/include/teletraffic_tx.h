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
       * Constructor
       */
      TeletrafficTx(std::string interface);
      
      /**
       * Destructor
       */
      ~TeletrafficTx();

      /**
       */
      int Init();
      
      /**
       *
       */
      int Run();
      
      /**
       *
       */
      bool IsRunning();
      
      /**
       *
       */
      int Stop();

      
private:

      std::string interface_;

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


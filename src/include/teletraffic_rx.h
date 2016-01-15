// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_TELETRAFFIC_TX_
#define TELETRAFFIC_TELETRAFFIC_TX_

#include <pthread.h>
#include <pcap/pcap.h>

#include <string>

namespace teletraffic {

      
      /**
       * Flow sink properties.
       */
      struct FlowSink {
            std::string sink_interface;
            int protocol_id;
            int packet_size;
            char source_mac[6];
      };
      
class TeletrafficTx {
public:

      
      /**
       * Constructor
       */
      TeletrafficTx();
      
      /**
       * Destructor
       */
      TeletrafficTx();

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

      /**
       */
      int sd();
      
private:
      pcap_t* pdev_;
      
      static
      void* ThreadFn(TeletrafficTx* obj);
      void* ThreadFn();
};

}

#endif // TELETRAFFIC_TELETRAFFIC_TX_


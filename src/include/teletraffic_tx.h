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
            std::string interface;
            int speed_bytes_per_second;
            int packet_protocol_id;
            int packet_size;
            char destination_mac[6];
      };
      

      /**
       * Transmitter statistics.
       */
      struct TxStatistics {

            // Network interface
            std::string interface;

            // Number of packets sent
            uint64_t sent_packet_count;
            
            // Protocol ID used in every sent packet
            uint16_t packet_protocol_id;
            
            // Size of sent packets
            uint16_t packet_size;
      };
      
      
      /**
       * Teletraffic transmitter.
       */
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

            /**
             * Returns a reference to a data struct which contains statistics abouts the transmitter.
             */
            const TxStatistics& Stats();      
      
      private:

            pthread_mutex_t lock_;
            pcap_t* txdev_;
            char* errbuf_;
            char* pkt_sent_;
            Stopwatch watch_;

            pthread_t thread_;
            pthread_attr_t thread_attr_;
            bool initialized_;
            bool thread_exit_;

            TxStatistics st_;
            TxStatistics st_copy_;

            static
            void* ThreadFn(void* obj);
            void* ThreadFn();
      };

}

#endif // TELETRAFFIC_TELETRAFFIC_TX_


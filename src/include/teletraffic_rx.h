// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_TELETRAFFIC_RX_
#define TELETRAFFIC_TELETRAFFIC_RX_

#include <string>
#include <pthread.h>
#include <pcap/pcap.h>
#include "stopwatch.h"
#include "utils.h"
#include "circular_buffer.h"
#include "service_thread.h"



namespace teletraffic {

      /**
       * Traffic receiver.
       */
      class TeletrafficRx : public ServiceThread {
      public:

            /**
             * Ctor.
             *
             * @param interface The network interface to listen to, e.g. "eth0", "eth1", etc.
             * @param protocol_id Value used in ethernet frame protocol field, e.g. 0xFF10.
             * @param window_duration Duration in microseconds of each time window. Default value is 1e6 us = 1 second.
             */
            TeletrafficRx(std::string interface, uint16_t protocol_id, uint64_t window_duration = 1e6);
      
            /**
             * Dtor.
             */
            ~TeletrafficRx();

            /**
             * Returns the data rate (in Mbps) measured at |window_index| window.
             */
            float RateMbpsAtWindow(int window_index);

            /**
             * Returns the data rate (in Mbps) averaged over the last |number_of_windows| windows. For example, if
             * number_of_windows = 10 and window_duration = 1 seconds then this value is the data rate averaged over the
             * last 10 seconds.
             */           
            float RateMbpsOverLast(int number_of_windows);

            /**
             * Number of windows whose statistics has been measured and stored. As the time passes this number will grow
             * from zero to 60, which is the maximum achievable.
             */
            int   WindowCount();

            /**
             * Textual description of the last fatal/internal error found. When the internal thread dies due to a fatal
             * error it leaves a message in an internal class variable before. Through this funcion you can view
             * that message.
             */
            const std::string& ErrorDescription() const { return last_error_; }



      private:
            /**
             * Statistics for 1 temporal window of 1 second.
             */
            struct Window {
                  Window() {
                        bytes = 0;
                        packets = 0;
                        duration = 0;
                        rate_Mbps = 0.0F;
                        rate_pps = 0.0F;
                  }
            
                  int bytes;
                  int packets;
                  int duration;
                  float rate_Mbps;
                  float rate_pps;
            };

      

            /**
             * General Statistics considering N seconds
             */
            struct RxStatistics {
                  RxStatistics() : window(60) { 
                        //interface = "";
                        packet_protocol_id = 0;
                        Reset(); 
                  }
                  void Reset() {
                        window.Clear();
                        recv_packet_count = 0;
                        last_packet_protocol = 0;
                        last_packet_size = 0;
                        last_packet_timestamp = 0;
                  }

                  // Network interface to sniff
                  std::string interface;
                  // Protocol ID used to filter incoming packets (used in PCAP filter expresion eth proto %d)
                  uint16_t packet_protocol_id;
            
                  // Rates in Mbps achieved over the last N seconds
                  CircularBuffer<Window> window;

                  // Total received packet counter (packets with protocol ID |packet_protocol_id|)
                  uint64_t recv_packet_count;

                  // Some properties of the last received packet
                  uint16_t last_packet_protocol;
                  uint16_t last_packet_size;
                  uint64_t last_packet_timestamp;
            };

            pthread_mutex_t lock_;

            uint64_t  window_duration_;
            
            pcap_t* rxdev_;
            std::string last_error_;

            Stopwatch watch_;
            uint64_t bytes_;
            uint64_t packets_;

            

            RxStatistics st_;
            
            uint8_t* pkt_recv_;
            uint64_t elapsed_;
            struct pcap_pkthdr header_;

            char* errbuf_;

            // Base class overrides
            int ServiceInit();
            int ServiceIteration();
      };
      
}

#endif  // TELETRAFFIC_TELETRAFFIC_RX_


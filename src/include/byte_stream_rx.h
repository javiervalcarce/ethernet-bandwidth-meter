// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_BYTE_STREAM_RX_H_
#define TELETRAFFIC_BYTE_STREAM_RX_H_

#include <stdint.h>
#include <string>
#include <pthread.h>
#include <cmath>

#include "service_thread.h"
#include "slip.h"
#include "stopwatch.h"
#include "circular_buffer.h"


namespace teletraffic {

      class ByteStreamRx : public ServiceThread {
      public:
            ByteStreamRx(int port, int window_duration_us);
            ~ByteStreamRx();

            double RateMbpsAtWindow(int window_index);
            double RateMbpsOverLast(int number_of_windows);
            int WindowCount();

      private:

            struct Window {
                  Window() {
                        bytes = 0;
                        packets = 0;
                        duration = 0;
                        rate_Mbps = nan("");
                        rate_pps = nan("");
                  }
            
                  uint64_t bytes;
                  uint64_t packets;
                  uint64_t duration;
                  double rate_Mbps;
                  double rate_pps;
            };


            pthread_mutex_t lock_;
            char buffer_[4096];
            static const int kReadTimeout = 10000;
            static const int kMaxReadTimeouts = 2000;

            int port_;
            int socket_fd_;
            int  timeout_count_;               // Number of read timeouts, after N server disconnects client
            bool first_;
            bool close_connection_;

            int         client_fd_;
            bool        client_connected_;      
            std::string client_ip_;
            int         client_port_;
            Stopwatch   watch_;

            uint64_t    acc_;
            uint64_t    window_duration_;    // Duration of a window
            uint64_t    bytes_;
            uint64_t    packets_;            // SLIP Frames

            // Rates in Mbps achieved over the last N seconds
            CircularBuffer<Window> window_;

            // Base class overrides
            int ServiceInitialize();
            int ServiceStart();
            int ServiceStop();
            int ServiceIteration();
            int ServiceFinalize();

      };
}

#endif   // TELETRAFFIC_BYTE_STREAM_RX_H_


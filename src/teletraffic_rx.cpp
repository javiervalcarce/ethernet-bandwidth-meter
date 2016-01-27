// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <string>

#include <pcap/pcap.h>

#include "teletraffic_rx.h"
#include "stopwatch.h"
#include "hexdump.h"

using namespace teletraffic;

const int PACKET_SIZE = 1024;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TeletrafficRx::TeletrafficRx(std::string interface) {

      pthread_mutex_init(&lock_, NULL);
      
      interface_ = interface;
      initialized_ = false;
      exit_thread_ = false;
      rxdev_ = NULL;
      
      errbuf_ = new char[PCAP_ERRBUF_SIZE];

      pthread_attr_init(&thread_attr_);
      pthread_attr_setdetachstate(&thread_attr_, PTHREAD_CREATE_JOINABLE);
      
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TeletrafficRx::~TeletrafficRx() {
      if (initialized_ == true) {
            exit_thread_ = true;
            //pthread_cancel(thread_);  
            pthread_join(thread_, NULL);
      }
   
      delete[] errbuf_;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TeletrafficRx::Init() {
      assert(initialized_ == false);
  
      errbuf_[0]='\0';

      // read timeout = 10 ms
      rxdev_ = pcap_open_live(interface_.c_str(), 64/*PACKET_SIZE*/, 1/*promisc*/, 10/*read timeout*/, errbuf_); 
      if (rxdev_ == NULL) {
            printf("rxdev_==NULL\n");
            return 1;
      }

      // Make sure we're capturing on an Ethernet device [2]
      // Warning: in MacOSX and BSD the loopack interface does not have Ethernet headers.
      if (pcap_datalink(rxdev_) != DLT_EN10MB) {
            printf("Not a IEEE 802.3 inteface\n");
            return 1;
      }

      //
      int r;
      r = pthread_create(&thread_, &thread_attr_, TeletrafficRx::ThreadFn, this);
      if (r != 0) {
            return 1;
      }
      initialized_ = true;
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Statistics& TeletrafficRx::Stats() {
      assert(initialized_ == true);

      // La estructura st está siendo constantemente actualizada por el hilo interno ThreadFn, para avitar condiciones
      // de carrera que den lugar la lecturas incorrectas de los valores por parte de aplicación garantizo la exlusión
      // mutua y hago una copia de la variable, esto es razonablemente eficiente ya que no pesa mucho (~70 bytes)
      pthread_mutex_lock(&lock_);
      st_copy_ = st_;
      pthread_mutex_unlock(&lock_);

      return st_copy_;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void* TeletrafficRx::ThreadFn(void* obj) {
      TeletrafficRx* o = (TeletrafficRx*) obj;
      return o->ThreadFn();
}
void* TeletrafficRx::ThreadFn() {

      watch_.Reset();
      watch_.Start();
      
      uint32_t pkt_1s_count = 0;
      uint32_t pkt_1s_bytes = 0;
      uint64_t pkt_1s_elaps = 0;        

      uint32_t pkt_4s_count = 0;
      uint32_t pkt_4s_bytes = 0;
      uint64_t pkt_4s_elaps = 0;

      uint32_t pkt_8s_count = 0;
      uint32_t pkt_8s_bytes = 0;
      uint64_t pkt_8s_elaps = 0;
      
      uint64_t us;
      int fsm = 0;

      bool pkt_first = true;
      uint64_t pkt_sequence_number = 0;

      st_.Reset();

      while (1) {

            if (exit_thread_ == true) {
                  break;
            }

            // Sniff - Minimal blocking call
            pkt_recv_ = (uint8_t*) pcap_next(rxdev_, &header_);            

            if (pkt_recv_ == NULL) {
                  // Read Timeout!
                  // There is no packet
            } else {


                  st_.recv_packet_count++; // atomic operation on x86_64 --> lock-free

                  st_.last_packet_protocol = *((uint16_t*) (pkt_recv_ + 12));
                  st_.last_packet_size = header_.len;
                  st_.last_packet_timestamp = header_.ts.tv_sec * 1e6 + header_.ts.tv_usec;

                  uint64_t sn = *((uint64_t*) (pkt_recv_ + 12 + 2 + 8));

                  if (pkt_first == true) {
                        pkt_first = false;
                        // Sync with this number
                        pkt_sequence_number = sn;
                  } else {
                        
                        pkt_sequence_number++; // expected
                        if (sn != pkt_sequence_number) {

                              // Resincronizo
                              printf("%llu != should %llu\n", sn, pkt_sequence_number);
                              st_.lost_packet_count += sn - st_.recv_packet_count;
                              pkt_sequence_number = sn;
                        }
                  }
                  
                  pkt_1s_count++;
                  pkt_4s_count++;
                  pkt_8s_count++;
                  
                  pkt_1s_bytes += header_.len;
                  pkt_4s_bytes += header_.len;
                  pkt_8s_bytes += header_.len;
            }



            us = watch_.ElapsedMicroseconds();            
            if (us > 1e6) {

                  pkt_1s_elaps += us;        
                  pkt_4s_elaps += us;        
                  pkt_8s_elaps += us;        

                  pthread_mutex_lock(&lock_);

                  // Estimación de la tasa
                  st_.rate_1s_pkps = (double) pkt_1s_count / ((double) pkt_1s_elaps / 1000000.0);
                  st_.rate_1s_Mbps = (double) pkt_1s_bytes / ((double) pkt_1s_elaps / 1000000.0) * 8.0 / 1000.0 / 1000.0;

                  pkt_1s_count = 0;
                  pkt_1s_bytes = 0;
                  pkt_1s_elaps = 0;

                  switch (fsm) {
                  case 0: fsm = 1; break;
                  case 1: fsm = 2; break; 
                  case 2: fsm = 3; break; 
                  case 3: fsm = 4;
                        st_.rate_4s_pkps = (double) pkt_4s_count / ((double) pkt_4s_elaps / 1000000.0);
                        st_.rate_4s_Mbps = (double) pkt_4s_bytes / ((double) pkt_4s_elaps / 1000000.0) * 8.0 / 1000.0 / 1000.0;
                        
                        pkt_4s_count = 0;
                        pkt_4s_bytes = 0;
                        pkt_4s_elaps = 0;
                        break;
                  case 4: fsm = 5; break;
                  case 5: fsm = 6; break;
                  case 6: fsm = 7; break;
                  case 7: fsm = 0;
                        st_.rate_8s_pkps = (double) pkt_8s_count / ((double) pkt_8s_elaps / 1000000.0);
                        st_.rate_8s_Mbps = (double) pkt_8s_bytes / ((double) pkt_8s_elaps / 1000000.0) * 8.0 / 1000.0 / 1000.0;
                        
                        pkt_8s_count = 0;
                        pkt_8s_bytes = 0;
                        pkt_8s_elaps = 0;
                        break;
                  };
                  
                  pthread_mutex_unlock(&lock_);

                  watch_.Reset();
            } 
      }

      watch_.Stop();
      pcap_close(rxdev_);
      return NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

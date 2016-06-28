// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <string>
#include <cmath>

#include <arpa/inet.h>
#include <pcap/pcap.h>


#include "teletraffic_rx.h"
#include "stopwatch.h"
#include "hexdump.h"

using namespace teletraffic;

const int PACKET_SIZE = 1024;
static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;  // mutex common for all class intances.


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TeletrafficRx::TeletrafficRx(std::string interface, uint16_t protocol_id, uint64_t window_duration) {
      pthread_mutex_init(&lock_, NULL);

      st_.interface = interface;
      st_.packet_protocol_id = protocol_id;
      window_duration_ = window_duration;

      rxdev_ = NULL;
      
      errbuf_ = new char[PCAP_ERRBUF_SIZE];
      errbuf_[0]='\0';

      st_.Reset();      

      bytes_ = 0;
      packets_ = 0;

      watch_.Reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TeletrafficRx::~TeletrafficRx() {
      if (rxdev_ != NULL) {
            pcap_close(rxdev_);
      }
   
      delete[] errbuf_;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TeletrafficRx::ServiceInit() {
      bpf_u_int32 mask;		// The netmask of our sniffing device
      bpf_u_int32 net;		// The IP of our sniffing device

      // La interfaz de red podía no estar activada (UP)
      char cmd[64];
      snprintf(cmd, sizeof(cmd), "ifconfig %s up", st_.interface.c_str());
      if (system(cmd) != 0) {
            printf("TeletrafficRx: Error executing this command: %s\n", cmd);
            return 1;
      }


      assert(st_.interface.length() > 0);
      assert(st_.packet_protocol_id != 0);

      if (pcap_lookupnet(st_.interface.c_str(), &net, &mask, errbuf_) == -1) {
            //printf("Can't get netmask for device %s\n", st_.interface.c_str());
            mask = 0;
            net = 0;
      }

      // read timeout = 10 ms
      rxdev_ = pcap_open_live(st_.interface.c_str(), 32/*64 PACKET_SIZE*/, 1/*promisc*/, 10/*read timeout*/, errbuf_); 
      if (rxdev_ == NULL) {
            printf("TeletrafficRx: rxdev_ is NULL\n");
            return 1;
      }

      // Make sure we're capturing on an Ethernet device [2]
      // Warning: in MacOSX and BSD the loopack interface does not have Ethernet headers.
      if (pcap_datalink(rxdev_) != DLT_EN10MB) {
            printf("TeletrafficRx: Not a IEEE 802.3 inteface\n");
            return 1;
      }

      // Sniff only INCOMING (received) packets.
      if (pcap_setdirection(rxdev_, PCAP_D_IN) == -1) {
            printf("TeletrafficRx: pcap_setdirection() error\n");
            return 1;
      }

      struct bpf_program fp;		                // The compiled filter expression
      //char filter_exp[] = "ether proto 65328";    // 0XFF30 The filter expression

      char filter_exp[80];    
      snprintf(filter_exp, sizeof(filter_exp), "ether proto %d", st_.packet_protocol_id);


      // WARNING: pcap_compile() **IS NOT** thread-safe => mutex
      pthread_mutex_lock(&s_lock);
      if (pcap_compile(rxdev_, &fp, filter_exp, 0, net) == -1) {
            printf("TeletrafficRx: Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(rxdev_));
            return 1;
      }
      pthread_mutex_unlock(&s_lock);

      if (pcap_setfilter(rxdev_, &fp) == -1) {
            printf("TeletrafficRx: Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(rxdev_));
            return 1;
      }
     

      // auto start
      watch_.Start();
      ServiceThread::Start();
      return 0;
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TeletrafficRx::ServiceIteration() {
      uint64_t us;

      // Sniff - Minimal blocking call
      pkt_recv_ = (uint8_t*) pcap_next(rxdev_, &header_);            

      if (pkt_recv_ == NULL) {
            // Read Timeout!
            // There is no packet... Nothing to do here.
      } else {

            pthread_mutex_lock(&lock_);
            st_.recv_packet_count++; // atomic operation on x86_64 --> lock-free

            //st_.last_packet_protocol = *((uint16_t*) (pkt_recv_ + 12)); // MAL
            st_.last_packet_protocol = ntohs(*((uint16_t*) (pkt_recv_ + 12)));
            st_.last_packet_size = header_.len;
            st_.last_packet_timestamp = header_.ts.tv_sec * 1e6 + header_.ts.tv_usec;
            
            packets_ += 1;
            bytes_ += header_.len;
            pthread_mutex_unlock(&lock_);
      }

      us = watch_.ElapsedMicroseconds();            
      if (us > window_duration_) {

            // Un nuevo elemento en la cola, si el número de elementos superase los 60 se sobreescribe el más
            // antiguo (vea la documentación de CircularBuffer<T>)
            Window w;
                  
            w.bytes     = bytes_;
            w.packets   = packets_;
            w.duration  = us;        
            w.rate_pps  = (float) w.packets / ((float) w.duration / 1000000.0F);
            w.rate_Mbps = (float) w.bytes   / ((float) w.duration / 1000000.0F) * 8.0F / 1000.0F / 1000.0F;

            pthread_mutex_lock(&lock_);
            st_.window.Push(w);
            pthread_mutex_unlock(&lock_);

            bytes_ = 0;
            packets_ = 0;
            watch_.Reset();
      } 

      return 0; // Call this function imediately (after 0 us)
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float TeletrafficRx::RateMbpsAtWindow(int second_index) {
      //assert(initialized_ == true);
      float v;
      pthread_mutex_lock(&lock_);
      if (second_index < st_.window.Size()) {
            v = st_.window[second_index].rate_Mbps;
      } else {
            v = nan("");
      }

      pthread_mutex_unlock(&lock_);
      return v;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float TeletrafficRx::RateMbpsOverLast(int number_of_windows) {
      //assert(initialized_ == true);
      assert(number_of_windows > 0);

      int i;
      int n;
      float v;

      n = number_of_windows;
      v = 0.0F;
      pthread_mutex_lock(&lock_);
      if (n <= st_.window.Size()) {
            for (i = 0; i < n; i++) {
                  v += st_.window[i].rate_Mbps;
            }
      } else {
            v = nan("");
      }
      pthread_mutex_unlock(&lock_);

      return v / n;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TeletrafficRx::WindowCount() {
      int n;
      pthread_mutex_lock(&lock_);
      n = st_.window.Size();
      pthread_mutex_unlock(&lock_);
      return n;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

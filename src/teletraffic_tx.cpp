// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include "teletraffic_tx.h"
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <arpa/inet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <sys/ioctl.h>

#include <pcap/pcap.h>

using namespace teletraffic;

const int PACKET_SIZE = 1024;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TeletrafficTx::TeletrafficTx(std::string interface, uint16_t protocol_id) {

      pthread_mutex_init(&lock_, NULL);

      st_.interface = interface;
      st_.sent_packet_count = 0;
      st_.packet_protocol_id = protocol_id;
      st_.packet_size = PACKET_SIZE;

      initialized_ = false;
      thread_exit_ = false;

      errbuf_   = new char[PCAP_ERRBUF_SIZE];
      pkt_sent_ = new char[PACKET_SIZE];

      pthread_attr_init(&thread_attr_);
      pthread_attr_setdetachstate(&thread_attr_, PTHREAD_CREATE_JOINABLE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TeletrafficTx::~TeletrafficTx() {
      delete[] errbuf_;
      delete[] pkt_sent_;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TeletrafficTx::Init() {

      assert(initialized_ == false);

      printf("sizeof(errbuf_)=%d\n", PCAP_ERRBUF_SIZE);
      printf("interface = %s\n", st_.interface.c_str());
      errbuf_[0] = '\0';
      txdev_ = pcap_open_live(st_.interface.c_str(), PACKET_SIZE, 0, 0, errbuf_);
      if (txdev_ == NULL) {
            printf("txdev_ NULL (%s)\n", errbuf_);
            return 1;
      }

      // Make sure we're capturing on an Ethernet device [2]
      // Warning: in MacOSX and BSD the loopack interface does not have Ethernet headers.
      if (pcap_datalink(txdev_) != DLT_EN10MB) {
            printf("Not IEEE 802.3 interface\n");
            return 1;
      }

      // Creación del hilo que lee y procesa las muestras de audio.
      int r;
      r = pthread_create(&thread_, &thread_attr_, TeletrafficTx::ThreadFn, this);
      if (r != 0) {
            return 1;
      }
      initialized_ = true;
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const TxStatistics& TeletrafficTx::Stats() {
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
void* TeletrafficTx::ThreadFn(void* obj) {
      TeletrafficTx* o = (TeletrafficTx*) obj;
      return o->ThreadFn();
}
void* TeletrafficTx::ThreadFn() {

      struct ether_header hdr;

/*
    // Obtain the source MAC address, copy into Ethernet header and ARP request.
    if (ioctl(fd,SIOCGIFHWADDR,&ifr)==-1) {
        perror(0);
        close(fd);
        exit(1);
    }
    if (ifr.ifr_hwaddr.sa_family!=ARPHRD_ETHER) {
        fprintf(stderr,"not an Ethernet interface");
        close(fd);
        exit(1);
    }
    const unsigned char* source_mac_addr=(unsigned char*)ifr.ifr_hwaddr.sa_data;

 */
      memset(hdr.ether_dhost, 0xFF, 6);
      memset(hdr.ether_shost, 0xA0, 6);
      hdr.ether_type = htons(st_.packet_protocol_id);

      memcpy(pkt_sent_, &hdr, sizeof(struct ether_header));

      char* payload = pkt_sent_ + sizeof(struct ether_header);

      payload[ 0]  = 'S';
      payload[ 1]  = 'E';
      payload[ 2]  = 'P';
      payload[ 3]  = 'S';
      payload[ 4]  = 'A';
      payload[ 5]  = '#';
      payload[ 6]  = '#';
      payload[ 7]  = '#';
      payload[ 8] = 0x00; // Número de secuencia (entero de 64 bits sin signo = uint64_t).
      payload[ 9] = 0x00;
      payload[10] = 0x00;
      payload[11] = 0x00;
      payload[12] = 0x00;
      payload[13] = 0x00;
      payload[14] = 0x00;
      payload[15] = 0x00; // Fin del número de secuencia.

      // Genero un vector de números pseudoaleatorios entre 0 y 255 ambos inclusive a partir del byte nº 10
      for (int i = 16; i < PACKET_SIZE; i++) {
            payload[i] = rand() % 256;
      }

      uint64_t  pkt_count = 0;
      uint64_t* pkt_count_p = (uint64_t*) &payload[8];
      uint64_t  error_count = 0;

      watch_.Reset();
      watch_.Start();

      while (1) {

            if (thread_exit_ == true) {
                  break;
            }

            // Envío una trama ethernet incrementando el número de secuencia.
            *pkt_count_p = pkt_count;

            int nb = pcap_inject(txdev_, pkt_sent_, PACKET_SIZE);
            if (nb != PACKET_SIZE) { 
                  error_count++;
                  usleep(200000); // 0,2 segundos - apaciguamiento de la generación de errores
            }

            pkt_count++;
            
            pthread_mutex_lock(&lock_);
            st_.sent_packet_count = pkt_count;
            pthread_mutex_unlock(&lock_);
           
      }

      return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

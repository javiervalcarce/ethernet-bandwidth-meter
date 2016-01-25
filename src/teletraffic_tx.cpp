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
TeletrafficTx::TeletrafficTx(std::string interface) {

      interface_ = interface;

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
      printf("interface = %s\n", interface_.c_str());
      errbuf_[0] = '\0';
      txdev_ = pcap_open_live(interface_.c_str(), PACKET_SIZE, 0, 0, errbuf_);
      if (txdev_ == NULL) {
            printf("txdev_ NULL (%s)\n", errbuf_);
            return 1;
      }


      // Make sure we're capturing on an Ethernet device [2] 
      // No, es contraproducente.
      //if (pcap_datalink(txdev_) != DLT_EN10MB) {
      //     return 1;
      //}

      // Creación del hilo que lee y procesa las muestras de audio.
      int r;
      r = pthread_create(&thread_, &thread_attr_, TeletrafficTx::ThreadFn, this);
      if (r != 0) {
            return 1;
      }

      return 0;
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
      hdr.ether_type = htons(0xFF30);

      memcpy(pkt_sent_, &hdr, sizeof(struct ether_header));

      char* payload = pkt_sent_ + sizeof(struct ether_header);
      
      payload[0] = 'S';
      payload[1] = 'E';
      payload[2] = 'P';
      payload[3] = 'S';
      payload[4] = 'A';
      payload[5] = '#';

      payload[6] = 0x00;
      payload[7] = 0x00;
      payload[8] = 0x00;
      payload[9] = 0x00;

      uint32_t* pkt_count_p = (uint32_t*) &payload[6];

      // Genero un vector de números pseudoaleatorios entre 0 y 255 ambos inclusive a partir del byte nº 10
      for (int i = 10; i < PACKET_SIZE; i++) {
            payload[i] = rand() % 256;
      }

      int pkt_count = 0;
      int error_count = 0;

      watch_.Reset();
      watch_.Start();

      while (1) {

            if (thread_exit_ == true) {
                  break;
            }

            // Envío una trama ethernet
            *pkt_count_p = pkt_count;

            int nb = pcap_inject(txdev_, pkt_sent_, PACKET_SIZE);
            if (nb != PACKET_SIZE) { 
                  error_count++;
                  printf("er\n");
                  usleep(100000); // 0,1 segundos
            }

            pkt_count++;
            if (pkt_count % 1000 == 0) {
                  printf("pkts = %d\n", pkt_count);
            }

      }

      return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

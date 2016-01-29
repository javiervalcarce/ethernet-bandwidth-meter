// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <string>
#include <getopt.h>
#include <unistd.h>

#include "teletraffic_tx.h"
#include "teletraffic_rx.h"


using namespace teletraffic;

std::string interface;
std::string mode;
std::string destination_mac;
std::string protocol_id_str;
int         protocol_id;


TeletrafficTx* tx = NULL;
TeletrafficRx* rx = NULL;


// tabla de opciones para getopt_long
/*
struct option long_options[] = {
      { "device",   required_argument, 0, 'd' },
      { "help",     no_argument,       0, 'h' },   
      { 0,          0,                 0,  0  }
};
*/

void Usage();
int TxMode();
int RxMode();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
      
      if (argc < 4) {
            Usage();
            return 0;
      }
      
      interface       = argv[1] != NULL ? argv[1] : "eth0";
      mode            = argv[2] != NULL ? argv[2] : "eth0";
      protocol_id_str = argv[3] != NULL ? argv[3] : "";
      destination_mac = argv[4] != NULL ? argv[4] : "FF:FF:FF:FF:FF:FF";

      char* endp;
      protocol_id = strtoul(protocol_id_str.c_str(), &endp, 0);
      if (*endp != '\0') {
            printf("Bad syntax for protocol_id [%s]\n", protocol_id_str.c_str());
            return 1;
      }

      printf("Using interface %s\n", interface.c_str());
      
      int err;
      if (mode == "tx") {
            err = TxMode();
      } else if (mode == "rx") {
            err = RxMode();
      } else {
            err = 1;
      }
      
      return err;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Usage() {
      printf("Usage: ./test_teletraffic <network-if> <tx|rx> <more-params>\n");
      printf("\n");
      printf("Examples:\n");
      printf("tx mode:  ./test_teletraffic <network-if> tx <protocol-id> <destination-mac>\n");
      printf("          ./test_teletraffic eth0 tx 0xFF30 FF:FF:FF:FF:FF:FF\n");
      printf("\n");
      printf("rx mode:  ./test_teletraffic <network-if> rx <protocol-id>\n");
      printf("          ./test_teletraffic eth0 rx 0xFF30\n");
      printf("\n");
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TxMode() {

      tx = new TeletrafficTx(interface, protocol_id);
      if (tx->Init() != 0) {
            printf("Initializing error. Are you root? Execute this program as root or with sudo.\n");
            return 1;
      }

      printf("Initialized\n");

      while (1) {
            
            const TxStatistics& s = tx->Stats();                        
            printf("sent pkts=%llu\n", s.sent_packet_count);
            usleep(500000); // 0,5 s
      }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int RxMode() {

      rx = new TeletrafficRx(interface, protocol_id);
      if (rx->Init() != 0) {
            printf("Initializing error. Are you root? Execute this program as root or with sudo.\n");
            return 1;
      }

      while (1) {

            const RxStatistics& s = rx->Stats();                        
            /*
            printf("rx=%llu, lost=%llu 1s=(%9.2f pkt/s, %7.2f Mbps) 4s=(%9.2f pkt/s, %7.2f Mbps) 8s=(%9.2f pkt/s, %7.2f Mbps)\n",
                   s.recv_packet_count,
                   s.lost_packet_count,
                   s.rate_1s_pkps,
                   s.rate_1s_Mbps, 
                   s.rate_4s_pkps,
                   s.rate_4s_Mbps,
                   s.rate_8s_pkps,
                   s.rate_8s_Mbps
                   );
            */
            printf("rx=%010llu, lost=%010llu 1s=%7.2f Mbps, 4s=%7.2f Mbps, 8s=%7.2f Mbps\n",
                   s.recv_packet_count,
                   s.lost_packet_count,
                   s.rate_1s_Mbps, 
                   s.rate_4s_Mbps,
                   s.rate_8s_Mbps
                   );
            
            usleep(500000); // 0.5 s
      }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

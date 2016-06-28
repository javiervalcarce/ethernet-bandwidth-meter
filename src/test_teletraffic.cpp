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
std::string source_mac;
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
      source_mac      = argv[5] != NULL ? argv[5] : "00:00:00:00:00:00";

      char* endp;
      protocol_id = strtoul(protocol_id_str.c_str(), &endp, 0);
      if (*endp != '\0') {
            printf("Bad syntax for protocol_id [%s]\n", protocol_id_str.c_str());
            return 1;
      }
      
      
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
      printf("tx mode:  test_teletraffic <network-if> tx <protocol-id> [<destination-mac>] [<source-mac>]\n");
      printf("               <destination-mac> is optional, if not specified the broadcast mac address will be used\n");
      printf("               <source-mac> is optional, if not specified the <network-if> mac address will be used\n");
      printf("\n");
      printf("               Example:\n");
      printf("               ./test_teletraffic eth0 tx 0xFF30 FF:FF:FF:FF:FF:FF 00:00:00:00:00:00\n");
      printf("\n");
      printf("rx mode:  test_teletraffic <network-if> rx <protocol-id>\n");
      printf("\n");
      printf("               Example:\n");
      printf("               ./test_teletraffic eth0 rx 0xFF30\n");
      printf("\n");
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TxMode() {

      uint8_t dst[6];
      uint8_t src[6];

      // Conversión de la MAC en formato cadena en array de 6 bytes
      MacAddressAsciiz2Binary(destination_mac.c_str(), dst);
      MacAddressAsciiz2Binary(source_mac.c_str(), src);
      
      if (source_mac != "00:00:00:00:00:00") {
            tx = new TeletrafficTx(interface, protocol_id, dst, src);
      } else {
            tx = new TeletrafficTx(interface, protocol_id, dst, NULL);
      }
      
      if (tx->Init() != 0) {
            printf("Initializing error. Are you root? Execute this program as root or with sudo.\n");
            return 1;
      }

      printf("Initialized.\n");
      //tx->Start();

      while (1) {
            const TxStatistics& s = tx->Stats();                        
            printf("Number of packets sent: %12llu packets\n", s.sent_packet_count);
            usleep(500000); // 0,5 s
      }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int RxMode() {
      int i;
      int n;

      rx = new TeletrafficRx(interface, protocol_id, 250000); // Ventanas de 0.25 segundos cada una
      if (rx->Init() != 0) {
            printf("Initializing error. Are you root? Execute this program as root or with sudo.\n");
            return 1;
      }

      //rx->Start();

      while (1) {
            
            n = rx->WindowCount();
            if (n > 30) {
                  n = 30;
            }
            
            for (i = 0; i < n; i++) {
                  printf("window[%02d] = %7.2f Mbps  AccumulatedMean = %7.2f Mbps\n", i, 
                         rx->RateMbpsAtWindow(i), 
                         rx->RateMbpsOverLast(1 + i));
            }
            
            usleep(250000); // 0.25 s

            printf("\r");
            for (i = 0; i < n; i++) {
                  printf("\x1b[A");
            }

      }

      delete rx;
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

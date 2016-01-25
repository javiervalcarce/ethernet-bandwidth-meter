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
std::string protocol_id;


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
      protocol_id     = argv[3] != NULL ? argv[3] : "";
      destination_mac = argv[4] != NULL ? argv[4] : "FF:FF:FF:FF:FF:FF";

      printf("Using interface %s\n", interface.c_str());
      
      if (mode == "tx") {
            TxMode();
      } else if (mode == "rx") {
            RxMode();
      } else {
            return 1;
      }
      
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Usage() {
      printf("Usage: ./test_teletraffic <network-if> <mode> ...\n");
      printf("tx mode:  ./test_teletraffic <network-if> tx <protocol-id> <destination-mac>\n");
      printf("rx mode:  ./test_teletraffic <network-if> rx <protocol-id>\n");
      printf("\n");
      printf("          ./test_teletraffic eth0 tx 0xFF30 FF:FF:FF:FF:FF:FF\n");
      printf("          ./test_teletraffic eth0 rx 0xFF30\n");
      printf("\n");
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TxMode() {

      tx = new TeletrafficTx(interface);
      
      if (tx->Init() != 0) {
            printf("Error initializing tx\n");
            return 1;
      }

      while (1) {
            usleep(100000); // 0,1 ms
      }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int RxMode() {

      rx = new TeletrafficRx(interface);

      if (rx->Init() != 0) {
            printf("Error initializing rx\n");
            return 1;
      }
      
      while (1) {
            usleep(100000); // 0,1 ms
      }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <string>
#include <getopt.h>
#include <unistd.h>

#include "teletraffic_tx.h"
#include "teletraffic_rx.h"
#include "byte_stream_tx.h"
#include "byte_stream_rx.h"

using namespace teletraffic;

std::string mode;
std::string hostname;
std::string port;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ByteStreamTx* tx = NULL;
ByteStreamRx* rx = NULL;

// tabla de opciones para getopt_long
/*
struct option long_options[] = {
      { "device",   required_argument, 0, 'd' },
      { "help",     no_argument,       0, 'h' },   
      { 0,          0,                 0,  0  }
};
*/

void Usage();
int TxMode(int argc, char** argv);
int RxMode(int argc, char** argv);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {

      if (argc < 3) {
            Usage();
            return 0;
      }
      
      mode = argv[1] != NULL ? argv[1] : "eth0";
      int err;
      if (mode == "tx") {
            err = TxMode(argc, argv);
      } else if (mode == "rx") {
            err = RxMode(argc, argv);
      } else {
            err = 1;
      }
      
      return err;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Usage() {
      printf("Usage: ./test_bs tx <ip-address> <port> | rx <port>\n");
      printf("\n");
      printf("Examples:\n");
      printf("rx mode:  test_bs rx 9009\n");
      printf("tx mode:  test_bs tx 192.168.1.2 9009\n");
      printf("\n");
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TxMode(int argc, char** argv) {

      hostname = argv[2] != NULL ? argv[2] : "127.0.0.1";
      port     = argv[3] != NULL ? argv[3] : "9009";

      int noport;
      char* endp;
      noport = strtoul(port.c_str(), &endp, 0);
      if (*endp != '\0') {
            printf("Bad syntax for port\n");
            return 1;
      }

      
      tx = new ByteStreamTx(hostname, noport);
      if (tx->Init() != 0) {
            printf("Initializing error\n");
            return 1;
      }

      printf("Tx Initialized.\n");      
      tx->Start();
      
      while (1) {
            //const TxStatistics& s = tx->Stats();                        
            printf("Number of bytes sent: %llu bytes\n", tx->SentByteCount());
            usleep(500000); // 0,5 s
      }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int RxMode(int argc, char** argv) {
      int i;
      int n;

      port = argv[2] != NULL ? argv[2] : "9009";

      int noport;
      char* endp;
      noport = strtoul(port.c_str(), &endp, 0);
      if (*endp != '\0') {
            printf("Bad syntax for port\n");
            return 1;
      }

      rx = new ByteStreamRx(noport, 1e6); // Ventanas de 1 segundos cada una
      if (rx->Init() != 0) {
            printf("Initializing error. Are you root? Execute this program as root or with sudo.\n");
            return 1;
      }
      
      printf("Rx Initialized.\n");
      rx->Start();
      
      n = rx->WindowCount();
      n = 16;

      while (1) {
            for (i = 0; i < n; i++) {
                  printf("window[%02d] = %10.2f Mbps  AccumulatedMean = %10.2f Mbps\n", i, 
                         rx->RateMbpsAtWindow(i), 
                         rx->RateMbpsOverLast(1 + i));
            }
            
            usleep(250000); 
            
            printf("\r");
            for (i = 0; i < n; i++) {
                  printf("\x1b[A");
            }
      }

      delete rx;
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

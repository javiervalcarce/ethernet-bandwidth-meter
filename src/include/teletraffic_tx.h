// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_TX_
#define TELETRAFFIC_TX_

#include <pthread.h>
#include <pcap/pcap.h>

//namespace eth_bandwidth_meter {

class DiagnoseETHCliente {
public:

      DiagnoseETHCliente(const char* eth);
      ~DiagnoseETHCliente();

      bool RunHiloEscritura();
      bool Enviando();

private:

      SockPacketRaw *ClienteRaw;
      bool enviando;
      bool EscribirDatos();
      static void *HiloEscritura(void *data);
};

//}

#endif // TELETRAFFIC_TX_


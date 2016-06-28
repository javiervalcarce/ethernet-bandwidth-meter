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
#include "netif_table.h"

using namespace teletraffic;

const int PACKET_SIZE = 1024;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TeletrafficTx::TeletrafficTx(std::string interface, uint16_t protocol_id, uint8_t* destination_mac, uint8_t* source_mac) {
      pthread_mutex_init(&lock_, NULL);
      source_.interface = interface;

      source_.packet_protocol_id = protocol_id;
      source_.speed_bytes_per_second = 0; // TODO: Implementar esto.
      source_.packet_size = PACKET_SIZE;
      
      //printf("Using interface %s\n", interface.c_str());

      // PPV: Para que el trafico atraviese un Acces Point en el caso de interfaces wifi, hay que poner una direccion MAC de origen
      // 20160330 JVG: Hago configurable la dirección mac origen
      if (source_mac != NULL) {
            memcpy(source_.source_mac, source_mac, 6);
      } else {
            // 20160506 JVG: Obtengo la MAC de la interfaz [interface] y uso esa

            std::map<std::string, NetworkInterface> table;
            NetworkInterfaceTableUpdate();
            NetworkInterfaceTable(&table);

            if (table.find(interface) != table.end()) {
                  memcpy(source_.source_mac, table.at(interface).mac_address, 6);
/*
                  printf("Using %02x:%02x:%02x:%02x:%02x:%02x as source mac\n", 
                         source_.source_mac[0], 
                         source_.source_mac[1], 
                         source_.source_mac[2], 
                         source_.source_mac[3], 
                         source_.source_mac[4], 
                         source_.source_mac[5]);
*/
            } else {

                  //printf("I couldn't get the mac adddress of the interface %s, I will use zeroes\n", interface.c_str());
                  // No he podido detectar dicha interfaz de red
                  memset(source_.source_mac, 0x00, 6);
            }
      }

      if (destination_mac != NULL) {
            memcpy(source_.destination_mac, destination_mac, 6);
      } else {
            memset(source_.destination_mac, 0xFF, 6);
      }

      st_.sent_packet_count = 0;
      txdev_ = NULL;
      errbuf_ = new char[PCAP_ERRBUF_SIZE];
      errbuf_[0] = '\0';
      pkt_sent_ = new char[source_.packet_size];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TeletrafficTx::~TeletrafficTx() {
      if (txdev_ != NULL) {
            pcap_close(txdev_);
      }

      delete[] errbuf_;
      delete[] pkt_sent_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TeletrafficTx::ServiceInit() {  // override
      // ONE-TIME INITIALIZATION
      char cmd[64];
      snprintf(cmd, sizeof(cmd), "ifconfig %s up", source_.interface.c_str());
      if (system(cmd) != 0) {
            printf("TeletrafficTx: Error executing this command: %s\n", cmd);
            return 1;
      }

      txdev_ = pcap_open_live(source_.interface.c_str(), source_.packet_size, 0, 0, errbuf_);
      if (txdev_ == NULL) {
            printf("TeletrafficTx: txdev_ is NULL (%s)\n", errbuf_);
            return 1;
      }

      // Make sure we're capturing on an Ethernet device [2]
      // Warning: in MacOSX and BSD the loopack interface does not have Ethernet headers.
      if (pcap_datalink(txdev_) != DLT_EN10MB) {
            printf("TeletrafficTx: Not an IEEE 802.3 interface\n");
            return 1;
      }


      // Este códig puede ir en el constructor
      // Relleno el paquete con una secuencia de números pseudoaleatorios entre 0 y 255 (ambos inclusive)
      for (int i = 0; i < source_.packet_size; i++) {
            pkt_sent_[i] = rand() % 256;
      }

      struct ether_header hdr;
      memcpy(hdr.ether_dhost, source_.destination_mac, 6);
      memcpy(hdr.ether_shost, source_.source_mac, 6);
      hdr.ether_type = htons(source_.packet_protocol_id);

      memcpy(pkt_sent_, &hdr, sizeof(struct ether_header));

      payload = pkt_sent_ + sizeof(struct ether_header);

      payload[ 0]  = 'S';
      payload[ 1]  = 'E';
      payload[ 2]  = 'P';
      payload[ 3]  = 'S';
      payload[ 4]  = 'A';
      payload[ 5]  = '#';
      payload[ 6]  = '#';
      payload[ 7]  = '#';

      payload[ 8] = 0x00;  // Número de secuencia (uint64_t).
      payload[ 9] = 0x00;
      payload[10] = 0x00;
      payload[11] = 0x00;
      payload[12] = 0x00;
      payload[13] = 0x00;
      payload[14] = 0x00;
      payload[15] = 0x00;  // Fin del número de secuencia (uint64_t).

      pkt_count_p = (uint64_t*) &payload[8];
      error_count = 0;
      pkt_count = 0;

      watch_.Reset();
      watch_.Start();

      // auto start
      ServiceThread::Start();
      return 0;
}
      
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TeletrafficTx::ServiceIteration() {  // override

      // Pongo en la trama el número de secuencia antes de enviarla
      *pkt_count_p = pkt_count;

      // Envío.
      int nb = pcap_inject(txdev_, pkt_sent_, source_.packet_size);
      if (nb != source_.packet_size) { 
            error_count++;
            usleep(200000); // 0,2 segundos - apaciguamiento de la generación de errores
      }

      // Incremento el número de secuencia.
      pkt_count++;

      pthread_mutex_lock(&lock_);     
      st_.sent_packet_count = pkt_count;
      pthread_mutex_unlock(&lock_);           

      return 0; // Call this function again inmediately (0 microseconds)
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const TxStatistics& TeletrafficTx::Stats() {
      // La estructura st está siendo constantemente actualizada por el hilo interno ThreadFn, para avitar condiciones
      // de carrera que den lugar la lecturas incorrectas de los valores por parte de aplicación garantizo la exlusión
      // mutua y hago una copia de la variable, esto es razonablemente eficiente ya que no pesa mucho (~70 bytes)
      pthread_mutex_lock(&lock_);
      st_copy_ = st_;
      pthread_mutex_unlock(&lock_);

      return st_copy_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

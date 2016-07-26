// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_TELETRAFFIC_TX_
#define TELETRAFFIC_TELETRAFFIC_TX_


#include <pthread.h>
#include <pcap/pcap.h>
#include <string>
#include "stopwatch.h"
#include "utils.h"
#include "service_thread.h"


namespace teletraffic {

      /**
       * Flow source properties
       */
      struct FlowSource {

            // Interfaz de red que se usará para enviar los paquetes.
            std::string interface;

            // Dirección que irá en los paquetes enviados, no tiene porqué ser la MAC de la intefaz [inteface], podría
            // ser distinta. Si se deja a ceros (00:00:00:00:00:00) entonces se usará la MAC de la interfaz [interface].
            uint8_t source_mac[6]; 

            // Velocidad en bytes por segundo. TODO: Implementar
            int speed_bytes_per_second;
            
            // Valor del campo protocol de los paquetes ethernet enviados.
            int packet_protocol_id;

            // Tamaño en bytes de los paquetes ethernet enviados.
            int packet_size;

            // Dirección destino del paquete. Para enviar paquetes broadcast use la dirección FF:FF:FF:FF:FF:FF
            uint8_t destination_mac[6];
      };
      


      /**
       * Transmitter statistics.
       */
      struct TxStatistics {
            TxStatistics() { 
                  Reset(); 
            }
            void Reset() {
                  sent_packet_count = 0;
            }

            // Number of packets sent
            uint64_t sent_packet_count;
      };
      
      
      /**
       * Teletraffic transmitter.
       *
       * This class represent a transmitter that sends raw ethernet (layer 2) packets at maximum speed using PCAP
       * library. Its main purpose is to send a labeled traffic flow that can be measured at the receiver end to
       * estimate network bandwidth.
       * 
       * Pcap library uses a special capture mechanism placed at kernel that let us send and receive (sniff) packet at
       * maximun speed. It's possible to measure with acuracy up to 10 Gbps links depending on the machine speed.
       */
      class TeletrafficTx : public ServiceThread {
      public:
            /**
             * Constructor.
             * 
             * @param interface Network interface used to transmit packets, every packet will [have protocol_id] in their
             * protocol field.
             *
             * @param protocol_id Protocol id for each packet.
             *
             * @param destination_mac Destination MAC address
             *
             * @param source_mac Source MAC address. If not specified the aunthentic MAC addres of the [interface]
             * network interface will be used.
             *
             */
            TeletrafficTx(std::string interface, uint16_t protocol_id, 
                          uint8_t* destination_mac = NULL, uint8_t* source_mac = NULL);

            /**
             * Destructor.
             */
            ~TeletrafficTx();

            /**
             * Returns a reference to a data struct which contains statistics about the transmitter.
             */
            const TxStatistics& Stats();      
      
      private:

            static const int kPacketSize = 1024;

            FlowSource source_;
            pthread_mutex_t lock_;
            pcap_t* txdev_;
            char* errbuf_;
            char* pkt_sent_;
            Stopwatch watch_;

            // todo: _
            char* payload;
            uint64_t* pkt_count_p;
            uint64_t  error_count;
            uint64_t  pkt_count;

            TxStatistics st_;
            TxStatistics st_copy_;

            // Base class overrides
            int ServiceInitialize();
            int ServiceStart();
            int ServiceStop();
            int ServiceIteration();
            int ServiceFinalize();
            
      };
}


#endif   // TELETRAFFIC_TELETRAFFIC_TX_


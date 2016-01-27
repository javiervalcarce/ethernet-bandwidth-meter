// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_TELETRAFFIC_RX_
#define TELETRAFFIC_TELETRAFFIC_RX_

#include <string>
#include <pthread.h>
#include <pcap/pcap.h>
#include "stopwatch.h"

namespace teletraffic {

      /**
       * Flow sink properties.
       */
      struct FlowSink {
            std::string sink_interface;
            int protocol_id;
            int packet_size;
            char source_mac[6];
      };

      
      /**
       * Statistics about received packets.
       */
      struct Statistics {
            Statistics() { Reset(); }
            void Reset() {
                  rate_1s_Mbps = 0.0;
                  rate_1s_pkps = 0.0;
                  rate_4s_Mbps = 0.0;
                  rate_4s_pkps = 0.0;
                  recv_packet_count = 0;
                  lost_packet_count = 0;
                  last_packet_protocol = 0;
                  last_packet_size = 0;
                  last_packet_timestamp = 0;
            }

            // Tasa instantanea (periodo de integración 1 segundo)
            double rate_1s_Mbps;
            double rate_1s_pkps;
            
            // Tasa instantanea (periodo de integración 4 segundo)
            double rate_4s_Mbps;
            double rate_4s_pkps;

            // Tasa instantanea (periodo de integración 4 segundo)
            double rate_8s_Mbps;
            double rate_8s_pkps;
                        
            // Contador total de paquetes recibidos.
            uint64_t recv_packet_count;

            // Número de paquetes perdidos (saltos de secuencia).
            uint64_t lost_packet_count;

            // Propiedades del último paquete recibido.
            uint16_t last_packet_protocol;
            uint16_t last_packet_size;
            uint64_t last_packet_timestamp;
      };


      /**
       * Traffic receiver.
       */
      class TeletrafficRx {
      public:

            /**
             * Constructor
             */
            TeletrafficRx(std::string interface);
      
            /**
             * Destructor
             */
            ~TeletrafficRx();

            /**
             * Inicialización.
             *
             * Configura el dispositivo ADC de captura y pone en marcha el hilo de procesado de señal.
             */
            int Init();
     
            /**
             * Devuelve un puntero a la estructura de datos que contiene las estadísticas recopiladas.
             */
            const Statistics& Stats();

            const std::string& ErrorDescription() const { return last_error_; }

      private:

            pthread_mutex_t lock_;

            std::string interface_;
            bool initialized_;
            bool exit_thread_;
            pcap_t* rxdev_;
            std::string last_error_;

            Stopwatch watch_;

            pthread_t thread_;
            pthread_attr_t thread_attr_;

            Statistics st_;
            Statistics st_copy_;
            
            uint8_t* pkt_recv_;
            uint64_t elapsed_;
            struct pcap_pkthdr header_;

            char* errbuf_;

            static
            void* ThreadFn(void* obj);
            void* ThreadFn();
      };
      
}

#endif // TELETRAFFIC_TELETRAFFIC_RX_


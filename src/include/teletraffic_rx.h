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
       * Traffic receiver.
       */
      class TeletrafficRx {
      public:

            /**
             * Statistics about received packets.
             */
            struct Statistics {
                  Statistics() {
                        Reset();
                  }

                  void Reset() {
                        rate_1_seconds = 0.0;
                        rate_4_seconds = 0.0;
                        rate_8_seconds = 0.0;
                        recv_packet_count = 0;
                        lost_packet_count = 0;
                        errorenous_packet_count = 0;
                        last_packet_protocol = 0;
                        last_packet_size = 0;
                        last_packet_timestamp = 0;
                  }

                  // Tasa instantanea (periodo de integración 1 segundo)
                  double rate_1_seconds;
            
                  // Tasa instantanea (periodo de integración 2 segundo)
                  double rate_4_seconds;
            
                  // Tasa instantanea (periodo de integración 8 segundo)
                  double rate_8_seconds;
            
                  // Contador total de paquetes recibidos.
                  int recv_packet_count;

                  // Número de paquetes perdidos (saltos de secuencia).
                  int lost_packet_count;

                  // Paquetes con errores en el tamaño (debería ser cero por el CRC32 de IEEE 802.3)
                  int errorenous_packet_count;

                  // Propiedades del último paquete recibido.
                  int last_packet_protocol;
                  int last_packet_size;
                  int last_packet_timestamp;
            };
      
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
             * Pone en marcha la recepción de paquetes y la obtención de estadísticas
             */
            int Run();
      
            /**
             * Informa de si el hilo de procesado de paquetes está funcionando o no.
             */
            bool IsRunning();
      
            /**
             * Detiene la recepción y procesado de paquetes.
             */
            int Stop();
     
            /**
             * Devuelve un puntero a la estructura de datos que contiene las estadísticas recopiladas.
             */
            const Statistics& Stat() const;
            
      private:

            std::string interface_;
            bool initialized_;
            bool exit_thread_;
            pcap_t* rxdev_;

            Stopwatch watch_;

            // Hilo
            pthread_t thread_;
            pthread_attr_t thread_attr_;

            Statistics st;

            u_char* str_sent;
            char* str_recv;
            
            const u_char *packet;
            struct pcap_pkthdr header;

            char* errbuf_;

            static
            void* ThreadFn(void* obj);
            void* ThreadFn();
      };

      
}

#endif // TELETRAFFIC_TELETRAFFIC_RX_


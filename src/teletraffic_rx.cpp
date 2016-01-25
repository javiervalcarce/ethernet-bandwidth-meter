// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <string>
#include <pcap/pcap.h>

#include "teletraffic_rx.h"
#include "stopwatch.h"

using namespace teletraffic;

const int PACKET_SIZE = 1024;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TeletrafficRx::TeletrafficRx(std::string interface) {
      
      interface_ = interface;
      initialized_ = false;
      exit_thread_ = false;
      rxdev_ = NULL;

      str_sent = new u_char[PACKET_SIZE];
      str_recv = new char[PACKET_SIZE];
      errbuf_  = new char[PCAP_ERRBUF_SIZE];

      memset(&thread_, 0, sizeof(pthread_t));

      pthread_attr_init(&thread_attr_);
      pthread_attr_setdetachstate(&thread_attr_, PTHREAD_CREATE_JOINABLE);
      //pthread_attr_getstacksize(&thread_attr_, &stacksize);
      
      
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TeletrafficRx::~TeletrafficRx() {
      //assert(initialized_ == true);

      exit_thread_ = true;
      //pthread_cancel(thread_);  
      pthread_join(thread_, NULL);
      
      if (rxdev_ != NULL) {
            //pcap_ter 
      }

      delete[] str_sent;
      delete[] str_recv;
      delete[] errbuf_;

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TeletrafficRx::Init() {
      assert(initialized_ == false);

      int nb;
      int count;
      int secs;

      std::ostringstream os;
      float rate;
  
      // RX
      errbuf_[0]='\0';
      rxdev_ = pcap_open_live(interface_.c_str(), PACKET_SIZE, 1, 0, errbuf_);
      if (rxdev_ == NULL) {
            return 1;
      }

      // Make sure we're capturing on an Ethernet device [2]
      if (pcap_datalink(rxdev_) != DLT_EN10MB) {
            return 1;
      }
      
      srand(time(NULL));


      // Creación del hilo que lee y procesa las muestras de audio.
      int r;
      r = pthread_create(&thread_, &thread_attr_, TeletrafficRx::ThreadFn, this);
      if (r != 0) {
            return 1;
      }
      initialized_ = true;
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TeletrafficRx::Run() {
      assert(initialized_ == true);     
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool TeletrafficRx::IsRunning() {
      assert(initialized_ == true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TeletrafficRx::Stop() {
      assert(initialized_ == true);     
      
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const TeletrafficRx::Statistics& TeletrafficRx::Stat() const {
      assert(initialized_ == true);
      return st;
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void* TeletrafficRx::ThreadFn(void* obj) {
      TeletrafficRx* o = (TeletrafficRx*) obj;
      return o->ThreadFn();
}
void* TeletrafficRx::ThreadFn() {

/*
      // Genero un vector de números pseudoaleatorios entre 0 y 255 ambos inclusive
      for (int i = 0; i < PACKET_SIZE; i++) {
            str_sent[i] = rand() % 256;
      }
*/

      watch_.Reset();
      watch_.Start();

      while (1) {

/*
            // Envío una trama ethernet
            nb = pcap_inject(pcap_tx, str_sent, sizeof(str_sent));
            if (nb != PACKET_SIZE){ 
                  ts->message = "Error to send";
                  goto sequence_9000_error_fatal;
            }
*/

            // Recibo la trama
            packet = (u_char*) pcap_next(rxdev_, &header);

            // Comprobación del tamaño
            if (header.len != PACKET_SIZE) { // Control que el tamaño recibido es el que queriamos al enviar
                  st.errorenous_packet_count++;
            }

            // Comparacion de los datos enviados y recibidos
            if (memcmp(str_sent, packet, PACKET_SIZE) != 0) {
                  st.errorenous_packet_count++;
            }

//count++;
      }

      return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

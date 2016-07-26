// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include "byte_stream_rx.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#include <cstring>
#include <cstdio>
#include <cassert>
#include <cmath>

using teletraffic::ByteStreamRx;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ByteStreamRx::ByteStreamRx(int port, int window_duration) : window_(60) {
      pthread_mutex_init(&lock_, NULL);
      
      port_ = port;
      socket_fd_ = -1;
      first_ = true;
      client_connected_ = false;
      close_connection_ = false;
      acc_ = 0;
      window_duration_ = window_duration;
      bytes_ = 0;
      packets_ = 0;
      timeout_count_ = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ByteStreamRx::~ByteStreamRx() {
      // NO ESCRIBIR NADA AQUÍ SALVO LLAMAR A Finalize() en la clase base. La liberación de recursos debe hacerse en
      // ServiceFinalize() y no aquí. Este destructor se ejecuta en un hilo DIFERENTE al del servicio.
      ServiceThread::Finalize();      
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ByteStreamRx::ServiceInitialize() {

      struct sockaddr_in servaddr;
      memset(&servaddr, 0, sizeof(servaddr));
      
      servaddr.sin_family = AF_INET;
      servaddr.sin_port = htons(port_);      
      servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
      //servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

      socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
      if (socket_fd_ == -1) {
            printf("ERROR en socket()\n");
            socket_fd_ = -1;
            return 1;
      }
      
      // Setup socket
      // SO_REUSEADDR permite reutilizar el puerto aunque haya sido usado previamente
      // SO_SIGNOPIPE evita que el sistem operativo envie este señal si la conexión tcp está rota
      int optval;
      optval = 1;      
      setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, (void*) &optval, sizeof(optval));

      signal(SIGPIPE, SIG_IGN);
      //optval = 1;      
      //setsockopt(socket_fd_, SOL_SOCKET, SO_SIGNOPIPE, (void*) &optval, sizeof(optval));

      int error;
      error = bind(socket_fd_, (struct sockaddr *) &servaddr, sizeof(servaddr));
      if (error != 0) {
            printf("ERROR en bind()\n");
            socket_fd_ = -1;
            return 1;
      }
      
      // En Linux 2.6+ el tamaño de la cola está limitado a 128 pongamos lo que pongamos, dejo 1024
      error = listen(socket_fd_, 1024);
      if (error != 0) {
            printf("ERROR en listen()\n");
            socket_fd_ = -1;
            return 1;
      }

      first_ = true;
      client_connected_ = false;
      close_connection_ = false;
      acc_ = 0;

      watch_.Reset();
      watch_.Start();
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ByteStreamRx::ServiceStart() {
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ByteStreamRx::ServiceStop() {
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ByteStreamRx::ServiceIteration() {

      struct sockaddr_in cliaddr;
      socklen_t clilen = sizeof(cliaddr);
      
      // Si es la primera iteración de este bucle o bien el cliente anterior ha indicado que cierre la conexión
      // entonces la cierro.
      if (first_ == true || close_connection_ == true) {

            // Cierro la conexión tcp con el cliente
            if (first_ == false) {
                  int er;
                  er = close(client_fd_);
                  if (er != 0) {
                        printf("ERROR en close()\n");
                  }
            }

            client_connected_ = false;
            close_connection_ = false;
            first_ = false;
            acc_ = 0;
            timeout_count_ = 0;
            window_.Clear();
            watch_.Stop();
            watch_.Reset();

            // Bloqueante.
            // Espero por la llegada de una conexión TCP...
            client_fd_ = accept(socket_fd_, (struct sockaddr *) &cliaddr, &clilen);
            if (client_fd_ == -1) {
                  // ¿qué podemos hacer aparte de imprimir una traza? Terminar el hilo...
                  printf("FATAL ERROR: accept()\n"); 
                  return -1;
            }
            
            // Deshabilita el algoritmo de Naggle
            //int flag = 1;
            //setsockopt(client_fd_, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));   

            struct timeval tv;
            tv.tv_sec  = 0;               //  Secs Timeout
            tv.tv_usec = kReadTimeout;    // uSecs Timeout

            setsockopt(client_fd_, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));

            //printf("Client connected!\n"); 
            watch_.Start();
            
            // TCP-KEEPALIVE O HTTP-KEEP ALIVE con temporización
            client_ip_ = inet_ntoa(cliaddr.sin_addr);
            client_port_ = ntohs(cliaddr.sin_port);
            client_connected_ = true;
            
      }

      int n;
      n = read(client_fd_, buffer_, sizeof(buffer_));
      if (n == 0) {
            //printf("TIMEOUT reading, maybe CLIENT HAS CLOSED connection\n");
            timeout_count_++;
            if (timeout_count_ > kMaxReadTimeouts) {
                  close_connection_ = true;      
            }
      } else if (n < 0) {
            // Error
            printf("ERROR reading from socket\n");
            close_connection_ = true;
            return 0;
      }
      
      acc_ += n;
      bytes_ += n;
      packets_ += 0;

      uint64_t us = watch_.ElapsedMicroseconds();            
      if (us > window_duration_) {

            // Un nuevo elemento en la cola, si el número de elementos superase los 60 se sobreescribe el más
            // antiguo (vea la documentación de CircularBuffer<T>)
            Window w;
                  
            w.bytes     = bytes_;
            w.packets   = packets_;
            w.duration  = us;        
            w.rate_pps  = (double) w.packets / ((double) w.duration / 1000000.0);
            w.rate_Mbps = (double) w.bytes   / ((double) w.duration / 1000000.0) * 8.0 / 1000.0 / 1000.0;

            pthread_mutex_lock(&lock_);
            window_.Push(w);
            pthread_mutex_unlock(&lock_);

            bytes_ = 0;
            packets_ = 0;
            watch_.Reset();
      } 

      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ByteStreamRx::ServiceFinalize() {
      close(client_fd_);
      close(socket_fd_);
      watch_.Stop();
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double ByteStreamRx::RateMbpsAtWindow(int window_index) {
      //assert(initialized_ == true);
      float v;
      pthread_mutex_lock(&lock_);
      if (window_index < window_.Size()) {
            v = window_[window_index].rate_Mbps;
      } else {
            v = nan("");
      }
      pthread_mutex_unlock(&lock_);
      return v;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double ByteStreamRx::RateMbpsOverLast(int number_of_windows) {
      //assert(initialized_ == true);
      assert(number_of_windows > 0);

      int i;
      int n;
      double v;

      n = number_of_windows;
      v = 0.0;
      pthread_mutex_lock(&lock_);
      if (n <= window_.Size()) {
            for (i = 0; i < n; i++) {
                  v += window_[i].rate_Mbps;
            }
      } else {
            v = nan("");
      }
      pthread_mutex_unlock(&lock_);

      return v / n;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ByteStreamRx::WindowCount() {
      return window_.Capacity();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


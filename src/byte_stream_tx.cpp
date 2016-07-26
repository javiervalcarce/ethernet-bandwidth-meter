// Hi Emacs, this is -*- coding: utf-8; mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include "byte_stream_tx.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

#include <cstdio>
#include <string>
#include <cstring>


using teletraffic::ByteStreamTx;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ByteStreamTx::ByteStreamTx(std::string hostname, int port) {
      hostname_ = hostname;
      port_ = port;
      framer_ = NULL;
      fd_ = -1;
      count_ = 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ByteStreamTx::~ByteStreamTx() {
      // NO ESCRIBA NADA AQUÍ SALVO UNA LLAMADA A Finalize() en la clase base. La liberación de recursos debe hacerse en
      // ServiceFinalize() y no aquí. Este destructor se ejecuta en un hilo DIFERENTE al hilo de servicio.
      ServiceThread::Finalize();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ByteStreamTx::ServiceInitialize() {
      struct sockaddr_in serv_addr;
      struct hostent* server;

      std::string prefix("/dev/tty");

      if (hostname_.compare(0, prefix.size(), prefix) == 0) {
            // Es un puerto serie, el nombre el fichero empieza por '/dev/tty'...
            fd_ = open(hostname_.c_str(), O_NOCTTY | O_WRONLY);  // O_RDWR
            if (fd_ < 0) {
                  printf("ERROR opening serial port\n");
                  return 1;
            }

            // Set up serial port speed.
            tcflush(fd_, TCIFLUSH);
            tcgetattr(fd_, &serial_conf_);
            cfmakeraw(&serial_conf_);
            cfsetspeed(&serial_conf_, B57600);
            tcsetattr(fd_, TCSANOW, &serial_conf_);
            
      } else {
            // Es una dirección IP, creo un socket AF_INET para conectar con el extremo remoto.
            fd_ = socket(AF_INET, SOCK_STREAM, 0);
            if (fd_ < 0) {
                  printf("ERROR opening socket\n");
                  return 1;
            }
            

            signal(SIGPIPE, SIG_IGN);

            // Esto solo funciona en BSD
            //int optval = 1;      
            //setsockopt(fd_, SOL_SOCKET, SO_SIGNOPIPE, (void*) &optval, sizeof(optval));


            server = gethostbyname(hostname_.c_str());
            if (server == NULL) {
                  printf("ERROR, no such host\n");
                  return 1;
            }
      
            bzero((char *) &serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    
            serv_addr.sin_port = htons(port_);
            if (connect(fd_, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
                  printf("ERROR connecting\n");
                  return 1;
            }
      }

      framer_ = new Slip(fd_);
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ByteStreamTx::ServiceStart() {
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ByteStreamTx::ServiceStop() {
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ByteStreamTx::ServiceIteration() {
      uint8_t buffer[4096];
      uint64_t n;

/*
  int r;
  r = framer_->Send(buffer, sizeof(buffer));
  if (r != 0) {
  printf("ERROR writing to socket\n");
  return -1;
  }
  // La trama ha sido enviada con éxito.
  count_ += sizeof(buffer);
*/
      
      n = write(fd_, buffer, sizeof(buffer));
      if (n < 0) {
            printf("ERROR writing to socket\n");
            return -1;  // Finish thread now, socket has been closed
      }
      count_ += n;
      // usleep(100000); // 100 ms

      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ByteStreamTx::ServiceFinalize() {
      if (framer_ != NULL) {
            delete framer_;
            framer_ = NULL;
      }

      if (fd_ != 1) {
            close(fd_);    
            fd_ = -1;
      }
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t ByteStreamTx::SentByteCount() {
      return count_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

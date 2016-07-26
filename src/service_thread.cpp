// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include "service_thread.h"
#include <cassert>
#include <cstdio>
#include <unistd.h>

using teletraffic::ServiceThread;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ServiceThread::ServiceThread(std::string thread_name) {

      internal_state_ = kNull;
      thread_name_ = thread_name;
      thread_exit_ = false;

      cmd_initialize_ = false;
      cmd_start_ = false;
      cmd_stop_ = false;
      cmd_finalize_ = false;

      pthread_attr_init(&thread_attr_);
      pthread_attr_setdetachstate(&thread_attr_, PTHREAD_CREATE_JOINABLE);  
      pthread_mutex_init(&lock_, NULL);
      
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ServiceThread::~ServiceThread() {
      //printf("ServiceThread::~ServiceThread()\n");
      if (internal_state_ == kNull) {
            return;
      }

      thread_exit_ = true;
      pthread_join(thread_, NULL);
      //printf("FIN.\n");
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ServiceThread::Init() {


      int r;
      r = pthread_create(&thread_, &thread_attr_, ServiceThread::ThreadFn, this);
      if (r != 0) {
            return 1;
      }

      //printf("ServiceThread::Init() 1\n");
      while (internal_state_ == kNull) {
            usleep(kIdleSleep);
            //pthread_yield();
      }

      
      //printf("ServiceThread::Init() 2\n");
      if (internal_state_ == kCrashed) {
            return 1;
      }

      //printf("ServiceThread::Init() 3\n");

      // Doy la orden de inicializar el servicio
      pthread_mutex_lock(&lock_);
      cmd_initialize_ = true;
      pthread_mutex_unlock(&lock_);

      while ((internal_state_ != kStopped) && (internal_state_ != kCrashed)) {
            //pthread_mutex_lock(&lock_);
            //printf("%s ## %s (this = %p)\n", thread_name_.c_str(), cmd_initialize_ ? "true" : "false", this);
            //pthread_mutex_unlock(&lock_);
            usleep(kIdleSleep);
      }

      if (internal_state_ == kCrashed) {
            //printf("Error while initializing service\n");
            return 1;
      }

      //printf("Successfully initialized\n");
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ServiceThread::Start() {
      if (internal_state_ == kNull) {
            return 1;
      }
      if (internal_state_ == kCrashed) {
            return 1;
      }

      pthread_t myself = pthread_self();
      if (pthread_equal(myself, thread_) != 0) {
            // Llamada desde el hilo de servicio. Error.
            //return 1;
            assert(false);
      }

      pthread_mutex_lock(&lock_);
      cmd_start_ = true;
      pthread_mutex_unlock(&lock_);

      while (internal_state_ != kStarted) {
            usleep(kIdleSleep);
      }
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ServiceThread::Stop() {
      if (internal_state_ == kNull) {
            return 1;
      }
      if (internal_state_ == kCrashed) {
            return 1;
      }

      pthread_t myself = pthread_self();
      if (pthread_equal(myself, thread_) != 0) {
            // Llamada desde el hilo de servicio. Error.
            //return 1;
            assert(false);
      }

      pthread_mutex_lock(&lock_);
      cmd_stop_ = true;
      pthread_mutex_unlock(&lock_);

      while (internal_state_ != kStopped) {
            usleep(kIdleSleep);
      }

      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ServiceThread::Finalize() {
      if (internal_state_ == kNull) {
            return 1;
      }
      if (internal_state_ == kCrashed) {
            return 1;
      }

      pthread_t myself = pthread_self();
      if (pthread_equal(myself, thread_) != 0) {
            // Llamada desde el hilo de servicio. Error.
            return 1;
            //assert(false);
      }

      pthread_mutex_lock(&lock_);
      cmd_finalize_ = true;
      pthread_mutex_unlock(&lock_);

      while (internal_state_ != kFinalized) {
            usleep(kIdleSleep);
      }
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void* ServiceThread::ThreadFn(void* obj) {
      ServiceThread* o = (ServiceThread*) obj;
      return o->ThreadFn();
}
void* ServiceThread::ThreadFn() {

      // Cambio el nombre de este hilo ejecutor de tareas por el de la nueva tarea que voy a ejecutar, esto
      // es EXTREMADAMENTE útil cuando depuramos el proceso con gdb (comando info threads)


#ifdef __linux__
      pthread_setname_np(thread_, thread_name_.c_str());
#else
      // Other POSIX
      pthread_setname_np(thread_name_.c_str());
#endif

      //printf("%s HILO: this = %p\n", thread_name_.c_str(), this);
      internal_state_ = kNotInitialized;

      int wait_for;
      int r;

      while (1) {

            if (thread_exit_) {
                  break;
            }

            switch (internal_state_) {
                  case kNull:
                        //printf("kNull\n");
                        // Este estado no puede ocurrir una vez create el hilo de servicio
                        assert(false);
                        break;

                  case kNotInitialized:
                        //printf("kNotInitialized\n");
                        
                        pthread_mutex_lock(&lock_);
                        //printf("%s >> %s (this = %p)\n", thread_name_.c_str(), cmd_initialize_ ? "true" : "false", this);
                        if (cmd_initialize_) {
                              internal_state_ = kInitializing;
                        } 
                        pthread_mutex_unlock(&lock_);

      
                        usleep(kIdleSleep);
                        break;

                  case kInitializing:
                        //printf("kInitializing\n");

                        pthread_mutex_lock(&lock_);
                        //cmd_initialize_ = false;
                        pthread_mutex_unlock(&lock_);

                        r = ServiceInitialize();
                        if (r != 0) {
                              internal_state_ = kCrashed;
                        } else {
                              internal_state_ = kStopped;
                        }
                        break;

                  case kStopping:
                        //printf("kStopping\n");

                        pthread_mutex_lock(&lock_);
                        cmd_stop_ = false;
                        pthread_mutex_unlock(&lock_);

                        r = ServiceStop();
                        internal_state_ = kStopped;
                        break;

                  case kStopped:
                        //printf("kStopped\n");

                        usleep(kIdleSleep);

                        pthread_mutex_lock(&lock_);
                        if (cmd_start_) {
                              internal_state_ = kStarting;
                        }
                        pthread_mutex_unlock(&lock_);

                        pthread_mutex_lock(&lock_);
                        if (cmd_finalize_) {
                              internal_state_ = kFinalizing;
                        }
                        pthread_mutex_unlock(&lock_);

                        break;

                  case kStarting:
                        //printf("kStarting\n");

                        pthread_mutex_lock(&lock_);
                        cmd_start_ = false;
                        pthread_mutex_unlock(&lock_);

                        r = ServiceStart();
                        internal_state_ = kStarted;
                        break;

                  case kStarted:
                        //printf("kStarted\n");

                        // Execute service
                        wait_for = ServiceIteration();
                        if (wait_for > 0) {
                              usleep(wait_for);
                        }

                        pthread_mutex_lock(&lock_);
                        if (cmd_stop_) {
                              internal_state_ = kStopping;
                        }
                        pthread_mutex_unlock(&lock_);


                        pthread_mutex_lock(&lock_);
                        if (cmd_finalize_) {
                              internal_state_ = kFinalizing;  // Más prioritario que STOP
                        }
                        pthread_mutex_unlock(&lock_);

                        break;

                  case kFinalizing:
                        //printf("kFinalizing\n");

                        pthread_mutex_lock(&lock_);
                        cmd_finalize_ = false;
                        pthread_mutex_unlock(&lock_);

                        r = ServiceFinalize();
                        internal_state_ = kFinalized;
                        break;

                  case kFinalized:
                        //printf("kFinalized\n");
                        usleep(kIdleSleep);
                        break;

                  case kCrashed:
                        //printf("kCrashed\n");
                        usleep(kIdleSleep);
                        break;

                  default:
                        assert(false);
                        break;
            }

      }

      return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

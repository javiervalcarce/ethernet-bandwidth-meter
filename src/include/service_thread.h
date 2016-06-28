// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_SERVICE_THREAD_H_
#define TELETRAFFIC_SERVICE_THREAD_H_
#include <pthread.h>

namespace teletraffic {

      /**
       * Base class.
       *
       * Continuosly calls its Worker function defined in a derived class and lets you start(), stop() the execution of
       * that function and query its execution state. It is a concept similar to a OS Service on Microsoft Windows
       * Operating Systems and a useful abstraction in multitude of contexts.
       */
      class ServiceThread {
      public:

            enum InternalState {
                  // Aun no se ha llamado a Init(), hasta que no llame a Init() no podra ponerlo en marcha con Start().
                  kNotInitialized,
                  // Parado. Puede ponerlo en marcha con Start().
                  kStopped,
                  // En marcha. Puede pararlo con Stop().
                  kRunning,
                  // Error fatal, el analizador esta parado, el hilo ha terminado, destruya el objeto (con delete) y 
                  // vuelva a instanciarlo.
                  kCrashed,
                  kFinished,
            };


            ServiceThread();
            virtual ~ServiceThread();
            int Init();
            
            int Start();
            int Stop();

      protected:
            virtual int ServiceInit();       // One-time (heavy) initialization. Free resource in ~Destructor of the derived class

            virtual int ServiceStart();      // Service is about to start (again)
            virtual int ServiceIteration();  // Service work, returned value is the number of microseconds to wait before call ServiceIteration() again.
            virtual int ServiceStop();       // Service is about to stop (again)

      private:
            pthread_t thread_;
            pthread_attr_t thread_attr_;
            pthread_mutex_t lock_;
            pthread_cond_t can_continue_;  

            bool initialized_;
            bool exit_thread_;
            InternalState internal_state_;

            static 
            void* ThreadFn(void* obj);
            void* ThreadFn();

      };

}


#endif  // TELETRAFFIC_SERVICE_THREAD_H_

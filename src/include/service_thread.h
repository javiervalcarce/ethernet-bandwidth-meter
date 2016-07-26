// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef TELETRAFFIC_SERVICE_THREAD_H_
#define TELETRAFFIC_SERVICE_THREAD_H_
#include <pthread.h>
#include <string>

namespace teletraffic {

      /**
       * ServiceThread Abstract Base Class.
       *
       * Continously calls its Worker function defined in a derived class and lets you start(), stop() the execution of
       * that function and query its execution state. It is a concept similar to a OS Service on Microsoft Windows
       * Operating Systems and a useful abstraction in multitude of contexts.
       */
      class ServiceThread {
      public:

            /**
             * Ctor.
             * @param thread_name The name of service thread, this name will show in a GDB session typing "info
             * threads" command.
             */
            ServiceThread(std::string thread_name = "");

            /**
             * Dtor.
             */
            virtual ~ServiceThread();


            int Init();            
            int Start();
            int Stop();

      protected:

            int Finalize();


            // One-time (heavy) initialization. Free the adquired resources at ServiceFinalize().
            // DO NOT USE DERIVED CLASS DESTRUCTOR FOR THAT PURPOSE.
            virtual int ServiceInitialize() =0;

            // Service is about to start, called always after a Start()
            virtual int ServiceStart() =0;

            // Service work, returned value is the number of microseconds to wait before call ServiceIteration()
            // again. If a negative number is returned then thread will be finished and status is kCrashed.
            virtual int ServiceIteration() =0;

            // Service is about to stop, called always after a Stop()
            virtual int ServiceStop() =0;

            // Service is about to be killed, any resource adquired in ServiceInit() should be freed here
            virtual int ServiceFinalize() =0;


      private:

            // State of the service thread.
            enum InternalState {

                  // Service thread is not yet created.
                  kNull = 0,

                  // An error happened during ServiceInitialization()
                  kCrashed,

                  // Not initialized yet. Call Init()
                  kNotInitialized,

                  // The ServiceInitialize() function is being called
                  kInitializing,

                  // The ServiceStop() function is being called
                  kStopping,

                  // Stopped, start it again with Start().
                  kStopped,

                  // The ServiceStart() function is being called
                  kStarting,

                  // Service is running, stop it again with Stop().
                  kStarted,

                  // The ServiceFinalize() function is being called
                  kFinalizing,

                  // Finished normally, this state is unrecoverable.
                  kFinalized
            };

            static const int kIdleSleep = 1000; // 1000 us = 1 ms

            pthread_t thread_;
            pthread_attr_t thread_attr_;

            pthread_mutex_t lock_;
            std::string thread_name_;
            bool thread_exit_;

            InternalState internal_state_;

            volatile bool cmd_initialize_;
            volatile bool cmd_start_;
            volatile bool cmd_stop_;
            volatile bool cmd_finalize_;

            static 
            void* ThreadFn(void* obj);
            void* ThreadFn();

      };

}


#endif  // TELETRAFFIC_SERVICE_THREAD_H_

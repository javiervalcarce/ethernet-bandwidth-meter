// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#include "service_thread.h"
#include <cassert>
#include <cstdio>
#include <unistd.h>

using teletraffic::ServiceThread;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ServiceThread::ServiceThread() {
      internal_state_ = kNotInitialized;
      exit_thread_ = false;
      pthread_attr_init(&thread_attr_);
      pthread_attr_setdetachstate(&thread_attr_, PTHREAD_CREATE_JOINABLE);  
      pthread_mutex_init(&lock_, NULL);
      pthread_cond_init(&can_continue_, NULL);    

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ServiceThread::~ServiceThread() {
      exit_thread_ = true;

      //pthread_cancel(thread_);  
      pthread_join(thread_, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ServiceThread::Init() {
      assert(internal_state_ == kNotInitialized);
      int r;

      internal_state_ = kStopped;     
      r = pthread_create(&thread_, &thread_attr_, ServiceThread::ThreadFn, this);
      if (r != 0) {
            internal_state_ = kNotInitialized;
            return 1;
      }

      r = ServiceInit();  // Derived class calling
      if (r != 0) {
            internal_state_ = kNotInitialized;
            return 1;
      }

      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ServiceThread::Start() {
      assert(internal_state_ != kNotInitialized);

      pthread_mutex_lock(&lock_);
      internal_state_ = kRunning;
      pthread_cond_signal(&can_continue_);
      pthread_mutex_unlock(&lock_);

      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ServiceThread::Stop() {
      assert(internal_state_ != kNotInitialized);

      pthread_mutex_lock(&lock_);
      internal_state_ = kStopped;
      pthread_cond_signal(&can_continue_);
      pthread_mutex_unlock(&lock_);

      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void* ServiceThread::ThreadFn(void* obj) {
      ServiceThread* o = (ServiceThread*) obj;
      return o->ThreadFn();
}
void* ServiceThread::ThreadFn() {

      int wait_for;

      while (1) {

            // ServiceStart(); // NOT IMPLEMENTED YET
            // ServiceStop();  // NOT IMPLEMENTED YET

            // Quedo a la espera de que la aplicación principal me permita continuar
            pthread_mutex_lock(&lock_);                  
            while (internal_state_ != kRunning) {
                  pthread_cond_wait(&can_continue_, &lock_);
            }
            pthread_mutex_unlock(&lock_);

            wait_for = ServiceIteration();
            
            if (wait_for > 0) {
                  usleep(wait_for);
            } else if (wait_for < 0) {
                  goto exit_error;
            } 

            // wait_for == 0 => no sleep
            if (exit_thread_ == true) {
                  goto exit_normal;
            }

      }


exit_normal:
      internal_state_ = kFinished;
      return NULL;
exit_error:
      internal_state_ = kCrashed;
      return NULL;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ServiceThread::ServiceInit() {
      printf("ServiceThread::ServiceInit\n");
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ServiceThread::ServiceStart() {
      printf("ServiceThread::ServiceStart\n");
      return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ServiceThread::ServiceIteration() {
      printf("ServiceThread::ServiceIteration\n");
      return 1e6;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ServiceThread::ServiceStop() {
      printf("ServiceThread::ServiceStop\n");
      return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

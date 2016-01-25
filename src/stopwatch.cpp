// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-

#include "stopwatch.h"

#include <time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

using namespace teletraffic;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Stopwatch::Stopwatch() {
      is_running_ = false;;   
      Reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t Stopwatch::ElapsedMicroseconds() { 

      if (is_running_ == false) {
            return elapsed_us_; 
      }

      time1_ = Timestamp();
      elapsed_us_ = time1_ - time0_;
//      elapsed_ms_ = elapsed_us_ / 1000;

      return elapsed_us_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t Stopwatch::ElapsedMilliseconds() { 
      if (is_running_ == false) {
            return elapsed_ms_; 
      }
      
      time1_ = Timestamp();

      elapsed_us_ = time1_ - time0_;
      elapsed_ms_ = elapsed_us_ / 1000;

      return elapsed_ms_;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string Stopwatch::CurrentTime() {

      char s[26];
      
      time_t v = time(NULL);
      ctime_r(&v, s);
      
      s[24] = '\0';
      
      return std::string(s);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Stopwatch::Start() {
      if (is_running_ == true) {
            return;
      }

      time0_ = Timestamp();

      elapsed_us_ = 0;
      elapsed_ms_ = 0;

      is_running_ = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Stopwatch::Stop() {
      if (is_running_ == false) {
            return; 
      }

      time1_ = Timestamp();

      elapsed_us_ = time1_ - time0_;
      elapsed_ms_ = elapsed_us_ / 1000;

      is_running_ = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Stopwatch::Reset() {

      time0_ = Timestamp();
      
      elapsed_ms_ = 0;
      elapsed_us_ = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t Stopwatch::Timestamp() {

      struct timespec ts;

#ifdef __MACH__
      // Esto es para MAC OS X
      clock_serv_t cclock;
      mach_timespec_t mts;

      host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);

      clock_get_time(cclock, &mts);
      mach_port_deallocate(mach_task_self(), cclock);
      ts.tv_sec = mts.tv_sec;
      ts.tv_nsec = mts.tv_nsec;
#else
      // Linux
      clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
#endif

      return (ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000); // en us


}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

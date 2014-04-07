/*  tripping-cyril
 *  Copyright (C) 2014  Toon Schoenmakers
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Timer.h"

#include <limits>

#include "Global.h"

namespace trippingcyril {

Timer::Timer(const Module* pModule)
: Event(pModule) {
  this->timer = NULL;
  this->stop = 0;
};

Timer::Timer(const Module* pModule, double interval, unsigned int maxCycles)
: Event(pModule) {
  this->timer = NULL;
  this->maxCycles = maxCycles;
  this->stop = 0;
  if (interval > 0) {
    if (maxCycles > 0)
      StartMaxCycles(interval, maxCycles);
    else
      Start(interval);
  };
};

Timer::~Timer() {
  if (timer != NULL)
    event_free(timer);
};

void Timer::Start(double interval) {
  struct timeval tv;
  tv.tv_sec = (__time_t) interval;
  tv.tv_usec = (__suseconds_t) ((interval - (double) tv.tv_sec) * 1000000.0);
  Start(tv);
};

void Timer::Start(struct timeval& tv) {
  if (timer == NULL) {
    timer = event_new(GetEventBase(), -1, EV_PERSIST, Timer::EventCallback, this);
    evtimer_add(timer, &tv);
  };
};

void Timer::StartMaxCycles(double interval, unsigned int maxCycles) {
  this->maxCycles = maxCycles;
  this->currentCycle = 0;
  Start(interval);
};

void Timer::Stop() {
  this->stop = 1;
  if (timer != NULL)
    event_free(timer);
  timer = NULL;
  Start(0.0);
};

void Timer::EventCallback(int fd, short int event, void* arg) {
  Timer* timer = (Timer*) arg;
  if (timer->stop == 1) {
    timer->Finished();
    delete timer;
  } else {
    timer->RunJob();
    if (timer->maxCycles > 0 && ++timer->currentCycle == timer->maxCycles) {
      timer->Finished();
      delete timer;
    };
  };
};

#if __cplusplus >= 201103

LamdbaTimer::LamdbaTimer(const Module* module, double interval, unsigned int maxCycles, const TimerLamdbaCallback& _callback)
: Timer(module, interval, maxCycles)
, callback(_callback) {
}

void LamdbaTimer::RunJob() {
  callback();
}

#endif

};
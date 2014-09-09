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
  namespace timing {

Timer::Timer(const Module* pModule)
: Event(pModule)
, _stop(0)
, _timer(NULL) {
};

Timer::Timer(const Module* pModule, double interval, unsigned int maxCycles)
: Event(pModule)
, _maxCycles(maxCycles)
, _stop(0)
, _timer(NULL) {
  if (interval > 0) {
    if (maxCycles > 0)
      StartMaxCycles(interval, maxCycles);
    else
      Start(interval);
  };
};

Timer::~Timer() {
  if (_timer != NULL)
    event_free(_timer);
};

void Timer::Start(double interval) {
  struct timeval tv;
  tv.tv_sec = (__time_t) interval;
  tv.tv_usec = (__suseconds_t) ((interval - (double) tv.tv_sec) * 1000000.0);
  Start(tv);
};

void Timer::Start(struct timeval& tv) {
  if (_timer == NULL) {
    _timer = event_new(GetEventBase(), -1, EV_PERSIST, Timer::EventCallback, this);
    evtimer_add(_timer, &tv);
  };
};

void Timer::StartMaxCycles(double interval, unsigned int maxCycles) {
  this->_maxCycles = maxCycles;
  this->_currentCycle = 0;
  Start(interval);
};

void Timer::Stop() {
  this->_stop = 1;
  if (_timer != NULL)
    event_free(_timer);
  _timer = NULL;
  Start(0.0);
};

void Timer::EventCallback(int fd, short int event, void* arg) {
  Timer* timer = (Timer*) arg;
  if (timer->_stop == 1) {
    timer->Finished();
    delete timer;
  } else {
    timer->RunJob();
    if (timer->_maxCycles > 0 && ++timer->_currentCycle == timer->_maxCycles) {
      timer->Finished();
      delete timer;
    };
  };
};

#if __cplusplus >= 201103

LamdbaTimer::LamdbaTimer(const Module* module, double interval, unsigned int maxCycles, const TimerLamdbaCallback& callback)
: Timer(module, interval, maxCycles)
, _callback(callback) {
}

LamdbaTimer::LamdbaTimer(const Module* module, const TimerLamdbaCallback& callback)
: Timer(module)
, _callback(callback) {
}

void LamdbaTimer::RunJob() {
  if (_callback)
    _callback();
}

#endif

  };
};
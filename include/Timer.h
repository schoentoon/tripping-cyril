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

#pragma once

#if __cplusplus >= 201103
#  include <functional>
#endif

#include <event2/event.h>

#include "Module.h"
#include "defines.h"

namespace trippingcyril {
  namespace timing {

/**
 * @brief General timer class
 */
class Timer : public Event {
public:
  /**
   * General constructor
   * @param module The module to register this timer on
   */
  Timer(const Module* module);
  /**
   * General constructor that will automatically start the timer
   * @param module The module to register this timer on
   * @param interval The interval this timer should run on
   * @param maxCycles The maximum amount of cycles this timer should run, 0 is infinite
   */
  Timer(const Module* module, double interval, unsigned int maxCycles = 0);
  /**
   * General deconstructor
   */
  virtual ~Timer();
  /**
   * Starts the timer with interval
   */
  void Start(double interval);
  /**
   * Starts the timer with tv as its interval
   */
  void Start(struct timeval& tv);
  /**
   * Starts the timer with interval, but it'll only run for maxCycles
   */
  void StartMaxCycles(double interval, unsigned int maxCycles);
  /**
   * Stops the timer and it'll get deleted as soon as possible.
   */
  void Stop();
protected:
  /**
   * Called on every interval
   */
  virtual void RunJob() = 0;
  /**
   * Called once our timer has ran its last cycle, will not get called if you
   * delete the timer yourself.
   */
  virtual void Finished() {};
private:
  // @cond
  unsigned int _maxCycles;
  unsigned int _currentCycle;
  unsigned char _stop : 1;
  struct event* _timer;
  static void EventCallback(evutil_socket_t fd, short event, void* arg);
  // @endcond
};

#if __cplusplus >= 201103

class LamdbaTimer : public Timer {
public:
  /**
   *  Typedef for the functions
   */
  typedef std::function<void()> TimerLamdbaCallback;
  /**
   * General constructor
   * @param module The module to register this timer on
   * @param interval The interval this timer should run on
   * @param maxCycles The maximum amount of cycles this timer should run, 0 is infinite
   * @param callback The function to execute every loop
   */
  LamdbaTimer(const Module* module, double interval, unsigned int maxCycles, const TimerLamdbaCallback &callback);
  /**
   * General constructor
   * @param module The module to register this timer on
   * @param callback The function to execute every loop
   */
  LamdbaTimer(const Module* module, const TimerLamdbaCallback &callback);
  /**
   *  Deconstructor
   */
  virtual ~LamdbaTimer() {};
protected:
  void RunJob() OVERRIDE;
private:
  const TimerLamdbaCallback _callback;
};

#endif

  };
};
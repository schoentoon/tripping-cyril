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

#ifndef _BACKOFF_TIMER_H
#define _BACKOFF_TIMER_H

#include "Timer.h"

namespace trippingcyril {

// @cond
class BackoffTimerImpl;
// @endcond

/**
 * @brief General backoff timer
 */
class BackoffTimer : public Event {
public:
  /**
   * General constructor
   * @param start_step At what interval should we start backing off?
   * @param step With what steps should we increase the interval
   * @param max_interval At what interval should we stop increasing the interval
   */
  BackoffTimer(const Module* module, double start_step = 15.0, double step = 15.0, double max_interval = 300.0);
  /** General deconstructor */
  virtual ~BackoffTimer();
  /** Call this to indicate you're still failing and we should continue backing off */
  void SetStillFailing();
protected:
  /**
   * Implement your actual logic here.
   */
  virtual void RunJob() = 0;
private:
  // @cond
  void ResetTimer();
  Timer* timer;
  double current_interval;
  double step;
  double max_interval;
  bool still_failing : 1;
  friend class BackoffTimerImpl;
  // @endcond
};

};

#endif //_BACKOFF_TIMER_H
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

#include "BackoffTimer.h"

namespace trippingcyril {
  namespace timing {

class BackoffTimerImpl : public Timer {
public:
  BackoffTimerImpl(BackoffTimer* timer, double interval)
  : Timer(timer->GetModule(), interval, 1) {
    this->timer = timer;
  };
  virtual ~BackoffTimerImpl() {
    timer->ResetTimer();
  };
protected:
  virtual void RunJob() {
    timer->RunJob();
  };
private:
  BackoffTimer* timer;
};

BackoffTimer::BackoffTimer(const Module* module, double start_step, double step, double max_interval)
: Event(module) {
  this->current_interval = start_step;
  this->step = step;
  this->max_interval = max_interval;
  still_failing = false;
  timer = new BackoffTimerImpl(this, start_step);
};

BackoffTimer::~BackoffTimer() {
  if (timer != NULL)
    delete timer;
};

void BackoffTimer::SetStillFailing() {
  if (timer == NULL)
    timer = new BackoffTimerImpl(this, current_interval);
  else
    still_failing = true;
};

void BackoffTimer::ResetTimer() {
  timer = NULL;
  current_interval += step;
  if (current_interval > max_interval)
    current_interval = max_interval;
  if (still_failing) {
    timer = new BackoffTimerImpl(this, current_interval);
    still_failing = false;
  };
};

  };
};
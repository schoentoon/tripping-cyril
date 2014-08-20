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
  : Timer(timer->GetModule(), interval, 1)
  , _timer(timer) {
  };
  virtual ~BackoffTimerImpl() {
    _timer->ResetTimer();
  };
protected:
  virtual void RunJob() {
    _timer->RunJob();
  };
private:
  BackoffTimer* _timer;
};

BackoffTimer::BackoffTimer(const Module* module, double start_step, double step, double max_interval)
: Event(module)
, _timer(new BackoffTimerImpl(this, start_step))
, _current_interval(start_step)
, _step(step)
, _max_interval(max_interval)
, _still_failing(false) {
};

BackoffTimer::~BackoffTimer() {
  if (_timer != NULL)
    delete _timer;
};

void BackoffTimer::SetStillFailing() {
  if (_timer == NULL)
    _timer = new BackoffTimerImpl(this, _current_interval);
  else
    _still_failing = true;
};

void BackoffTimer::ResetTimer() {
  _timer = NULL;
  _current_interval += _step;
  if (_current_interval > _max_interval)
    _current_interval = _max_interval;
  if (_still_failing) {
    _timer = new BackoffTimerImpl(this, _current_interval);
    _still_failing = false;
  };
};

  };
};
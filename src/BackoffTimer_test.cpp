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

#include <gtest/gtest.h>

#include "LibEventHelper.h"

#include "BackoffTimer.h"

namespace test {

class BackoffTimer : public LibEventTest {
};

class BackoffTimerTest : public trippingcyril::BackoffTimer {
public:
  BackoffTimerTest(trippingcyril::Module* pModule, double start_step, double step, double max_interval)
  : trippingcyril::BackoffTimer(pModule, start_step, step, max_interval) {
    counter = 0;
    stopAt = 0;
    done = NULL;
  };
  unsigned int counter;
  unsigned int stopAt;
  bool *done;
protected:
  void RunJob() {
    counter++;
    if (stopAt > 0 && stopAt == counter) {
      if (done)
        *done = true;
      event_base_loopbreak(GetModule()->GetEventBase());
    } else
      SetStillFailing();
  };
};

TEST_F(BackoffTimer, Backoff) {
  BackoffTimerTest* timer = new BackoffTimerTest(event_base, 0.01, 0.01, 0.2);
  bool done = false;
  timer->done = &done;
  timer->stopAt = 5;
  while (done == false && event_base->Loop());
  EXPECT_TRUE(done);
  EXPECT_EQ(5, timer->counter);
  delete timer;
};

};
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

#include "Timer.h"

namespace test {

class Timer : public LibEventTest {
};

class TimerTest : public trippingcyril::Timer {
public:
  TimerTest(trippingcyril::Module* pModule)
  : trippingcyril::Timer(pModule) {
    counter = 0;
    stopAt = 0;
    done = NULL;
    finished = NULL;
  };
  unsigned int counter;
  unsigned int stopAt;
  bool *done;
  bool *finished;
protected:
  void RunJob() {
    counter++;
    if (stopAt > 0 && stopAt == counter) {
      if (done)
        *done = true;
      Stop();
    };
  };
  void Finished() {
    if (done)
      *done = true;
    if (finished)
      *finished = true;
    event_base_loopbreak(GetModule()->GetEventBase());
  };
};

TEST_F(Timer, StartMaxCycle) {
  TimerTest* timer = new TimerTest(event_base);
  timer->StartMaxCycles(0.0001, 100);
  bool done = false;
  timer->done = &done;
  while (done == false && event_base->Loop());
  EXPECT_TRUE(done);
  EXPECT_EQ(100, timer->counter);
  EXPECT_DEATH(delete timer, "");
};

TEST_F(Timer, Start) {
  TimerTest* timer = new TimerTest(event_base);
  timer->stopAt = 2;
  timer->Start(0.0001);
  bool done = false;
  timer->done = &done;
  bool finished = false;
  timer->finished = &finished;
  while (done == false && finished == false && event_base->Loop());
  EXPECT_TRUE(done);
  EXPECT_TRUE(finished);
  EXPECT_EQ(2, timer->counter);
  EXPECT_DEATH(delete timer, "");
};

#if __cplusplus >= 201103

TEST_F(Timer, Lamdba) {
  bool done = false;
  int counter = 0;
  LamdbaTimer* timer = new LamdbaTimer(event_base, 0.0001, 2, [&done,&counter,this]() {
    if (++counter == 2) {
      done = true;
      event_base_loopbreak(event_base->GetEventBase());
    };
  });
  while (done == false && event_base->Loop());
  EXPECT_TRUE(done);
  EXPECT_EQ(2, counter);
  EXPECT_DEATH(delete timer, "");
}

#endif

};
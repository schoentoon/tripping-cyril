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

#include "JobThread.h"

#include "LibEventHelper.h"

using namespace trippingcyril;

namespace test {

class JobThread : public LibEventTest {
};

class TestJob : public Job {
public:
  TestJob(Module* module) {
    this->done = NULL;
    this->module = module;
    this->mainThread = 0;
    this->counter = 0;
  };
  virtual ~TestJob() {
  };
  virtual void preExecuteMain() {
    if (mainThread != 0) {
      EXPECT_EQ(mainThread, pthread_self());
    }
    EXPECT_EQ(0, counter++);
  };
  virtual void execute() {
    if (mainThread != 0) {
      EXPECT_NE(mainThread, pthread_self());
    }
    EXPECT_EQ(1, counter++);
  };
  virtual void postExecuteMain() {
    if (mainThread != 0) {
      EXPECT_EQ(mainThread, pthread_self());
    }
    EXPECT_EQ(2, counter++);
    if (done != NULL)
      *done = true;
    event_base_loopbreak(module->GetEventBase());
  };
  bool* done;
  Module* module;
  pthread_t mainThread;
  int counter;
};

TEST_F(JobThread, TestJob) {
  trippingcyril::JobThread* thread = new trippingcyril::JobThread("jobthread", event_base);
  ASSERT_TRUE(thread->Start());
  TestJob* job = new TestJob(event_base);
  job->mainThread = pthread_self();
  bool done = false;
  job->done = &done;
  thread->Add(job);
  while (done == false && event_base->Loop());
  delete thread;
};

};
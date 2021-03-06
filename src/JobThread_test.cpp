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
using namespace trippingcyril::thread;

namespace test {

class JobThread : public LibEventTest {
public:
  JobThread()
  : LibEventTest() {
    timeout = 1;
  };
};

class TestJob : public Job {
public:
  TestJob(Module* module)
  : Job() {
    this->done = NULL;
    this->module = module;
    this->mainThread = 0;
    this->counter = 0;
    this->shouldRun = true;
  };
  virtual ~TestJob() {
    if (shouldRun && runPostHook) {
      EXPECT_EQ(3, counter);
    } else if (shouldRun == false) {
      EXPECT_EQ(1, counter);
    } else if (runPostHook == false) {
      EXPECT_EQ(2, counter);
    };
  };
  virtual bool preExecuteMain() {
    if (mainThread != 0) {
      EXPECT_EQ(mainThread, pthread_self());
    }
    EXPECT_EQ(0, counter++);
    if (shouldRun == false) {
      if (done != NULL)
        *done = true;
      event_base_loopbreak(module->GetEventBase());
    };
    return shouldRun;
  };
  virtual void execute() {
    if (mainThread != 0) {
      EXPECT_NE(mainThread, pthread_self());
    }
    EXPECT_EQ(1, counter++);
    if (runPostHook == false && done != NULL)
      *done = true;
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
  bool shouldRun;
};

TEST_F(JobThread, DISABLED_TestJob) {
  trippingcyril::thread::JobThread* thread = new trippingcyril::thread::JobThread("jobthread", event_base);
  ASSERT_TRUE(thread->Start());
  TestJob* job = new TestJob(event_base);
  job->mainThread = pthread_self();
  bool done = false;
  job->done = &done;
  thread->Add(job);
  while (done == false && event_base->Loop());
  EXPECT_DEATH(delete job, "");
};

TEST_F(JobThread, DISABLED_NoPostHook) {
  trippingcyril::thread::JobThread* thread = new trippingcyril::thread::JobThread("jobthread", event_base);
  ASSERT_TRUE(thread->Start());
  TestJob* job = new TestJob(event_base);
  job->runPostHook = false;
  job->mainThread = pthread_self();
  bool done = false;
  job->done = &done;
  thread->Add(job);
  while (done == false && event_base->Loop());
  EXPECT_DEATH(delete job, "");
};

TEST_F(JobThread, DISABLED_DontRun) {
  trippingcyril::thread::JobThread* thread = new trippingcyril::thread::JobThread("jobthread", event_base);
  ASSERT_TRUE(thread->Start());
  TestJob* job = new TestJob(event_base);
  job->shouldRun = false;
  job->mainThread = pthread_self();
  bool done = false;
  job->done = &done;
  thread->Add(job);
  while (done == false && event_base->Loop());
  EXPECT_DEATH(delete job, "");
};

};
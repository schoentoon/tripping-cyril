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

#include "JobThread.h"

#include <iostream>

namespace trippingcyril {

class JobRunnerPipe : public Pipe {
public:
  JobRunnerPipe(const Module* pModule, JobThread* pThread)
  : Pipe(pModule) {
    this->thread = pThread;
  };
  virtual ~JobRunnerPipe() {
  };
  struct JobRunner {
    char mode;
    Job* job;
  };
  virtual void OnRead() {
    JobRunner jobr;
    while (Read((char*) &jobr, sizeof(jobr)) == sizeof(jobr) && jobr.job != NULL) {
      switch (jobr.mode) {
      case 0:
        jobr.job->preExecuteMain();
        thread->jobs.push_back(jobr.job);
        thread->condvar->Signal();
        break;
      case 1:
        jobr.job->postExecuteMain();
        delete jobr.job;
        break;
      };
    };
  };
private:
  JobThread* thread;
};

JobThread::JobThread(const String& pName, const Module* pModule)
: Thread(pName)
, module(pModule) {
  mutex = new Mutex;
  condvar = new CondVar;
  pipe = new JobRunnerPipe(pModule, this);
};

JobThread::~JobThread() {
  delete mutex;
  delete condvar;
  delete pipe;
};

void JobThread::Add(Job* job) {
  MutexLocker lock(mutex);
  JobRunnerPipe::JobRunner runner;
  runner.mode = 0;
  runner.job = job;
  pipe->Write((const char*) &runner, sizeof(runner));
};

Job* JobThread::Remove() {
  MutexLocker lock(mutex);
  while (jobs.size() == 0)
    condvar->Wait(mutex);
  Job* job = jobs.front();
  jobs.pop_front();
  return job;
};

void* JobThread::run() {
  while (shouldContinue()) {
    Job* job = Remove();
    if (job == NULL)
      break;
    job->execute();
    JobRunnerPipe::JobRunner runner;
    runner.mode = 1;
    runner.job = job;
    pipe->Write((const char*) &runner, sizeof(runner));
  };
  return NULL;
};

};
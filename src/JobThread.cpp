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
  namespace thread {

class JobRunnerPipe : public Pipe {
public:
  JobRunnerPipe(JobThread* thread)
  : Pipe(thread->GetModule())
  , _thread(thread) {
  };
  virtual ~JobRunnerPipe() {
  };
  struct JobRunner {
    char mode;
    Job* job;
  };
protected:
  virtual void OnRead() {
    JobRunner jobr;
    while (Read((char*) &jobr, sizeof(jobr)) == sizeof(jobr) && jobr.job != NULL) {
      switch (jobr.mode) {
      case 0:
      case 1:
        if (jobr.job->preExecuteMain()) {
          if (jobr.mode == 0)
            _thread->jobs.push_back(jobr.job);
          else if (jobr.mode == 1)
            _thread->jobs.push_front(jobr.job);
          _thread->condvar->Signal();
        } else {
          if (jobr.job->shouldDelete())
            delete jobr.job;
        };
        break;
      case 2:
        if (jobr.job->runPostHook)
          jobr.job->postExecuteMain();
        if (jobr.job->shouldDelete())
          delete jobr.job;
        break;
      };
    };
  };
private:
  JobThread* _thread;
};

JobThread::JobThread(const String& pName, const Module* pModule)
: Thread(pName, pModule) {
  mutex = new Mutex;
  condvar = new CondVar;
  pipe = new JobRunnerPipe(this);
};

JobThread::~JobThread() {
  delete mutex;
  delete condvar;
  delete pipe;
};

void JobThread::Add(Job* job) {
  if (job == NULL)
    return;
  MutexLocker lock(mutex);
  JobRunnerPipe::JobRunner runner;
  runner.mode = 0;
  runner.job = job;
  pipe->Write((const char*) &runner, sizeof(runner));
};

void JobThread::AddFront(Job* job) {
  if (job == NULL)
    return;
  MutexLocker lock(mutex);
  JobRunnerPipe::JobRunner runner;
  runner.mode = 1;
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
    if (job->runPostHook) {
      JobRunnerPipe::JobRunner runner;
      runner.mode = 2;
      runner.job = job;
      pipe->Write((const char*) &runner, sizeof(runner));
    } else if (job->shouldDelete())
      delete job;
  };
  return NULL;
};

  };
};

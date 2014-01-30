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

#ifndef _JOB_THREAD_H
#define _JOB_THREAD_H

#include <list>

#include "Thread.h"
#include "Module.h"
#include "Pipe.h"

namespace trippingcyril {

/**
 * @brief Job base class
 * With main thread we really just mean the thread the Module that the JobThread
 * we are pushed to is running on.
 */
class Job {
public:
  /** Deconstructor */
  virtual ~Job() {};
  /**
   * This method will be executed on the 'main' thread before execution
   */
  virtual void preExecuteMain() = 0;
  /**
   * This method will run on the JobThread
   */
  virtual void execute() = 0;
  /**
   * This method will run on the 'main' thread after execution of execute()
   */
  virtual void postExecuteMain() = 0;
};

// @cond
class JobRunnerPipe;
// @endcond

/**
 * @brief A job queue that will run your jobs on a seperate thread
 */
class JobThread : public Thread {
public:
  /** Constructor */
  JobThread(const String& pName, const Module* pModule);
  /** Deconstructor */
  virtual ~JobThread();
  /** Add a new job to your job queue */
  void Add(Job* job);
  /** Get the module this thread was created with */
  const Module* GetModule() const { return module; };
protected:
  Job* Remove();
  void* run();
private:
  // @cond
  const Module* module;
  list<Job*> jobs;
  Mutex* mutex;
  CondVar* condvar;
  Pipe* pipe;
  friend class JobRunnerPipe;
  // @endcond
};

};

#endif //_JOB_THREAD_H
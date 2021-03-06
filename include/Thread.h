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

#pragma once

#include <map>
#include <stdint.h>
#include <pthread.h>

#include "String.h"
#include "Event.h"

namespace trippingcyril {
  namespace thread {

/**
 * @brief Simple thread class
 */
class Thread : public Event {
public:
  /**
   * Standard constructor
   * @param pModule The module to register this thread on
   * @param pName The name the thread will get
   */
  Thread(const String& pName, const Module* pModule);
  /**
   * Standard deconstructor
   */
  virtual ~Thread();
  /**
   * @brief Starts the thread
   * @return True if thread succesfully started
   */
  bool Start();
  /**
   * @brief Does a join operation on the thread
   * @return True if succesfull
   */
  bool Join();
  /**
   * @brief Does a detach operation on the thread
   * @return True if succesfull
   */
  bool Detach();
  /**
   * @brief Get the internal thread id
   * @return The pthread id of the running thread, 0 if not yet running
   */
  pthread_t Self() const { return _tid; };
  /**
   * @brief Tells the thread to stop
   * @see shouldContinue
   */
  void Stop();
  /**
   * @brief See if the thread is running or not
   * @return True if the thread is running
   */
  bool isRunning() const { return _running == 1; };
  /** @brief Receive the value returned from the thread after a Join operation */
  void* getReturnedValue() const { return _returnedValue; };
  /** @brief Receive the name of the thread */
  const String GetName() const { return _name; };
protected:
  /**
   * @brief The method that is actually executed on the thread, override this
   */
  virtual void* run() = 0;
  /**
   * @brief Receive a hint about what you should do, continue or not? Depends mostly
   * on the internal stop value set by Stop
   * @return True if you should not stop
   */
  bool shouldContinue() const;
private:
  // @cond
  static void* runThread(void* arg);
  pthread_t _tid;
  uint8_t _running : 1;
  uint8_t _detached : 1;
  const String _name;
  void* _returnedValue;
  pthread_mutex_t* _mutex;
  // @endcond
};

class CondVar;

/**
 * @brief Simple mutex class
 */
class Mutex {
public:
  /** Constructor */
  Mutex();
  /** Deconstructor */
  virtual ~Mutex();
  /** @return True if mutex is succesfully locked */
  bool Lock();
  /** @return True if mutex is succesfully unlocked */
  bool Unlock();
  /** @return True if mutex is locked */
  bool isLocked();
private:
  // @cond
  pthread_mutex_t _mutex;
  friend class CondVar;
  // @endcond
};

/**
 * @brief Simple mutexlocking class
 */
class MutexLocker {
public:
  /**
   * Constructor that locks the mutex
   */
  MutexLocker(Mutex* mutex)
  : _mutex(mutex) {
    if (_mutex != NULL)
      _mutex->Lock();
  };
  /**
   * Deconstructor that unlocks the mutex
   */
  virtual ~MutexLocker() {
    if (_mutex != NULL)
      _mutex->Unlock();
  };
private:
  // @cond
  Mutex* _mutex;
  // @endcond
};

/**
 * @brief Simple cond var wrapping class
 */
class CondVar {
public:
  /** Constructor */
  CondVar();
  /** Deconstructor */
  virtual ~CondVar();
  /**
   * Signals any waiting operations
   * @return True if succesful
   */
  bool Signal();
  /**
   * Waits for any signals
   */
  bool Wait(Mutex* mutex);
private:
  // @cond
  pthread_cond_t _cond_var;
  // @endcond
};

/**
 * @brief A thread manager to easily be able to get the Thread object of your
 * current thread.
 */
class ThreadManager {
public:
  /** @return The singleton instance of this class */
  static ThreadManager* Get();
  /** @return The Thread* object of this current thread, NULL on error (main thread) */
  Thread* getCurrentThread() const;
  /** @return The amount of threads managed by this ThreadManager */
  size_t threadCount() const;
private:
  // @cond
  ThreadManager();
  virtual ~ThreadManager();
  void registerThread(Thread* thread);
  void unregisterThread(Thread* thread);

  std::map<pthread_t, Thread*> _threads;
  Mutex* _lock;
  friend class Thread;
  // @endcond
};

  };
};

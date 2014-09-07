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

#include "Thread.h"

#include <stdlib.h>
#include <sys/prctl.h>
#include <stdio.h>

namespace trippingcyril {
  namespace thread {

Thread::Thread(const String& name, const Module* module)
: Event(module)
, _tid(0)
, _running(0)
, _detached(0)
, _name(name)
, _returnedValue(NULL)
, _mutex((pthread_mutex_t*) malloc(sizeof(pthread_mutex_t))) {
  pthread_mutex_init(_mutex, NULL);
};

Thread::~Thread() {
  Stop();
  if (_running == 1 && _detached != 1)
    pthread_detach(_tid);
  if (_running == 1)
    pthread_cancel(_tid);
  pthread_mutex_destroy(_mutex);
  free(_mutex);
};

void* Thread::runThread(void* arg) {
  Thread* thread = static_cast<Thread*>(arg);
  if (thread) {
    prctl(PR_SET_NAME,thread->_name.c_str(),0,0,0);
    ThreadManager::Get()->registerThread(thread);
    void* output = thread->run();
    ThreadManager::Get()->unregisterThread(thread);
    return output;
  };
  perror("Thread::runThread(), what the hell is going on here? This shouldn't be executed!");
  return NULL;
};

bool Thread::Start() {
  int result = pthread_create(&_tid, NULL, Thread::runThread, this);
  if (result == 0) {
    _running = 1;
    pthread_mutex_lock(_mutex);
  };
  return result == 0;
};

bool Thread::Join() {
  int result = -1;
  if (_running == 1) {
    result = pthread_join(_tid, &_returnedValue);
    if (result == 0)
      _detached = 1;
  }
  return result == 0;
};

bool Thread::Detach() {
  int result = -1;
  if (_running == 1 && _detached == 0) {
    result = pthread_detach(_tid);
    if (result == 0)
      _detached = 1;
  }
  return result == 0;
};

bool Thread::shouldContinue() const {
  if (_mutex == NULL)
    return false;
  if (pthread_mutex_trylock(_mutex) == 0) {
    pthread_mutex_unlock(_mutex);
    return false;
  };
  return true;
};

void Thread::Stop() {
  if (_running == 1)
    pthread_mutex_unlock(_mutex);
};

Mutex::Mutex() {
  pthread_mutex_init(&_mutex, NULL);
};

Mutex::~Mutex() {
  pthread_mutex_destroy(&_mutex);
};

bool Mutex::Lock() {
  return pthread_mutex_lock(&_mutex) == 0;
};

bool Mutex::Unlock() {
  return pthread_mutex_unlock(&_mutex) == 0;
};

bool Mutex::isLocked() {
  if (pthread_mutex_trylock(&_mutex) == 0) {
    pthread_mutex_unlock(&_mutex);
    return false;
  };
  return true;
};

CondVar::CondVar() {
  pthread_cond_init(&_cond_var, NULL);
};

CondVar::~CondVar() {
  pthread_cond_destroy(&_cond_var);
};

bool CondVar::Signal() {
  return pthread_cond_signal(&_cond_var) == 0;
};

bool CondVar::Wait(Mutex* mutex) {
  return pthread_cond_wait(&_cond_var, &mutex->_mutex) == 0;
};

ThreadManager::ThreadManager()
: _threads()
, _lock(new Mutex()) {
};

ThreadManager::~ThreadManager() {
  delete _lock;
};

ThreadManager* ThreadManager::Get() {
  static ThreadManager* singleton = new ThreadManager;
  return singleton;
};

Thread* ThreadManager::getCurrentThread() const {
  MutexLocker locker(_lock);
  std::map<pthread_t, Thread*>::const_iterator iter = _threads.find(pthread_self());
  if (iter != _threads.end())
    return iter->second;
  return NULL;
};

size_t ThreadManager::threadCount() const {
  MutexLocker locker(_lock);
  return _threads.size();
};

void ThreadManager::registerThread(Thread* thread) {
  MutexLocker locker(_lock);
  if (thread->isRunning())
    _threads.insert(std::pair<pthread_t, Thread*>(thread->Self(), thread));
};

void ThreadManager::unregisterThread(Thread* thread) {
  MutexLocker locker(_lock);
  pthread_t id = thread->Self();
  std::map<pthread_t, Thread*>::iterator iter = _threads.find(id);
  if (iter != _threads.end())
    _threads.erase(iter);
};

  };
};

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

Thread::Thread(const String& pName)
: name(pName) {
  tid = 0;
  running = 0;
  detached = 0;
  returnedValue = NULL;
  mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(mutex, NULL);
};

Thread::~Thread() {
  Stop();
  if (running == 1 && detached != 1)
    pthread_detach(tid);
  if (running == 1)
    pthread_cancel(tid);
  pthread_mutex_destroy(mutex);
  free(mutex);
};

void* Thread::runThread(void* arg) {
  Thread* thread = static_cast<Thread*>(arg);
  if (thread) {
    prctl(PR_SET_NAME,thread->name.c_str(),0,0,0);
    return thread->run();
  };
  perror("Thread::runThread(), what the hell is going on here? This shouldn't be executed!");
  return NULL;
};

bool Thread::Start() {
  int result = pthread_create(&tid, NULL, Thread::runThread, this);
  if (result == 0) {
    running = 1;
    pthread_mutex_lock(mutex);
  };
  return result == 0;
};

bool Thread::Join() {
  int result = -1;
  if (running == 1) {
    result = pthread_join(tid, &returnedValue);
    if (result == 0)
      detached = 1;
  }
  return result == 0;
};

bool Thread::Detach() {
  int result = -1;
  if (running == 1 && detached == 0) {
    result = pthread_detach(tid);
    if (result == 0)
      detached = 1;
  }
  return result == 0;
};

bool Thread::shouldContinue() {
  if (mutex == NULL)
    return false;
  if (pthread_mutex_trylock(mutex) == 0) {
    pthread_mutex_unlock(mutex);
    return false;
  };
  return true;
};

pthread_t Thread::Self() {
  return tid;
};

void Thread::Stop() {
  if (running == 1)
    pthread_mutex_unlock(mutex);
};

};
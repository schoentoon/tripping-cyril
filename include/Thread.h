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
#ifndef _THREAD_H
#define _THREAD_H

#include <stdint.h>
#include <pthread.h>

#include "String.h"

namespace trippingcyril {

/**
 * @brief Simple thread class
 */
class Thread {
public:
  /**
   * Standard constructor
   * @param pName The name the thread will get
   */
  Thread(const String& pName);
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
  pthread_t Self();
  /**
   * @brief Tells the thread to stop
   * @see shouldContinue
   */
  void Stop();
  /**
   * @brief See if the thread is running or not
   * @return True if the thread is running
   */
  bool isRunning() { return running == 1; };
  /**
   * @brief Receive the value returned from the thread after a Join operation
   */
  void* getReturnedValue() { return returnedValue; };
  /**
   * @brief Receive the name of the thread
   */
  const String GetName() const { return name; };
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
  bool shouldContinue();
private:
  // @cond
  static void* runThread(void* arg);
  pthread_t  tid;
  uint8_t running : 1;
  uint8_t detached : 1;
  const String name;
  void* returnedValue;
  pthread_mutex_t* mutex;
  // @endcond
};

};

#endif //_THREAD_H
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

#ifndef _PIPE_H
#define _PIPE_H

#include <event2/event.h>

#include "Files.h"
#include "Writer.h"
#include "Module.h"
#include "Thread.h"

namespace trippingcyril {

/**
 * @brief A wrapper class for pipes
 */
class Pipe : public Event, public Writer {
public:
  /**
   * General constructor
   * @param pModule The module to register this pipe on
   */
  Pipe(const Module* pModule = NULL);
  /**
   * Deconstructor, will close both ends of the pipe
   */
  virtual ~Pipe();
  /**
   * Generic read operation from this pipe, similar to
   * <a href="http://man7.org/linux/man-pages/man2/read.2.html">read(2)</a>
   */
  int Read(char* buffer, size_t len) {
    return read(fds[0], buffer, len);
  };
  /**
   * Generic write operation to this pipe, similar to
   * <a href="http://man7.org/linux/man-pages/man2/write.2.html">write(2)</a>
   */
  int Write(const char* buffer, size_t len) {
    MutexLocker lock(&mutex);
    return write(fds[1], buffer, len);
  };
  /**
   * Override this method with your own stuff to do when read operations happen
   */
  virtual void OnRead() = 0;
private:
  // @cond
  int fds[2];
  struct event* read_event;
  Mutex mutex;
  static void EventCallback(evutil_socket_t fd, short event, void* arg);
  // @endcond
};

};

#endif //_PIPE_H
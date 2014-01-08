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

#ifndef _TIMER_H
#define _TIMER_H

#include <event2/event.h>

#include "Module.h"

namespace trippingcyril {

class Timer {
public:
  Timer(Module* module);
  Timer(Module* module, double interval, unsigned int maxCycles = 0);
  virtual ~Timer();
  void Start(double interval);
  void Start(struct timeval& tv);
  void StartMaxCycles(double interval, unsigned int maxCycles);
  void Stop();
  Module* GetModule() const { return module; };
protected:
  virtual void RunJob() {};
  virtual void Finished() {};
private:
  Module* module;
  unsigned int maxCycles;
  unsigned int currentCycle;
  unsigned char stop : 1;
  struct event* timer;
  static void EventCallback(evutil_socket_t fd, short event, void* arg);
};

};

#endif //_TIMER_H
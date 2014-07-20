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

#include <event2/event.h>

#include "Module.h"

namespace trippingcyril {

#include "Event.h"

class TriggerEvent : public Event {
public:
  TriggerEvent(const Module* pModule = NULL);
  virtual ~TriggerEvent();
  void Trigger();
  virtual void OnTrigger() {};
private:
  // @cond
  struct event* event;
  static void EventCallback(evutil_socket_t fd, short event, void* arg);
  // @endcond
};

};
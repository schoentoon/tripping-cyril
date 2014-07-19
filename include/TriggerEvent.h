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
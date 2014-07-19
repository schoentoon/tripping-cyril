#include "TriggerEvent.h"

#include "Module.h"

namespace trippingcyril {

TriggerEvent::TriggerEvent(const Module* pModule)
: Event(pModule) {
  event = event_new(GetEventBase(), -1, EV_PERSIST, EventCallback, this);
};

TriggerEvent::~TriggerEvent() {
  event_free(event);
};

void TriggerEvent::Trigger() {
  event_active(event, 0, 0);
};

void TriggerEvent::EventCallback(evutil_socket_t fd, short event, void* arg) {
  TriggerEvent* trigger = (TriggerEvent*) arg;
  trigger->OnTrigger();
};

};
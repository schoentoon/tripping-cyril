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

#include "Event.h"

#include "Module.h"
#include "Global.h"

namespace trippingcyril {

Event::Event(const Module* pModule)
: module(pModule) {
  if (module != NULL)
    module->AddEvent(this);
};

Event::~Event() {
  if (module != NULL)
    module->DelEvent(this);
};

event_base* Event::GetEventBase() const {
  return (module != NULL) ? module->GetEventBase() : Global::Get()->GetEventBase();
};

evdns_base* Event::GetDNSBase() const {
  return (module != NULL) ? module->GetDNSBase() : Global::Get()->GetDNSBase();
};


};
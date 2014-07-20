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

#include <event2/dns.h>
#include <event2/event.h>

namespace trippingcyril {

// @cond
class Module;
// @endcond

/**
 * @brief Event class to automatically register and unregister event driven classes
 * with modules, simply implement this from any event driven class
 */
class Event {
public:
  /** General constructor that registers with the module, may be NULL
   * @param module The module to register this event on
   */
  Event(const Module* module);
  /** General deconstructor that unregisters from the module */
  virtual ~Event();
  /**
   * Used to get the module used to initialize this event
   */
  const Module* GetModule() const { return module; };
protected:
  /** No need to keep track of the module yourself in your class, just access this
   * one
   */
  const Module* module;
  event_base* GetEventBase() const;
  evdns_base* GetDNSBase() const;
};

};
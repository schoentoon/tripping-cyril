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

#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <event2/dns.h>
#include <event2/event.h>

namespace trippingcyril {

class Global {
public:
  static Global* Get() {
    static Global* singleton = new Global;
    return singleton;
  };
  struct event_base* GetEventBase() { return event_base; };
  struct evdns_base* GetDNSBase() { return dns_base; };
  void Loop();
private:
  Global();
  virtual ~Global();

  struct event_base* event_base;
  struct evdns_base* dns_base;
};

};

#endif //_GLOBAL_H
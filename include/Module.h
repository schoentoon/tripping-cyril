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

#ifndef _MODULE_H
#define _MODULE_H

#include <event2/dns.h>
#include <event2/event.h>
#include <set>

#include "String.h"
#include "Global.h"

#define MODCONSTRUCTOR(CLASS) \
  CLASS::CLASS(ModHandle so, const String& modName) \
  : trippingcyril::Module(so, modName)

#define MODCONSTRUCTORHEADER(CLASS) \
  CLASS(ModHandle so, const String& modName)

#define MODULEDEFS(CLASS) \
  extern "C" { \
    Module* ModLoad(ModHandle so, const String& modName) { \
      return new CLASS(so, modName); \
    }; \
  };

namespace trippingcyril {

typedef void* ModHandle;

class Socket;
class Timer;

class Module {
public:
  Module(ModHandle so, const String& modName);
  virtual ~Module();
  virtual String GetVersion() const = 0;
  virtual void OnLoaded() {};
  const String GetModName() const { return modName; };
  struct event_base* GetEventBase() const { return event_base; };
  struct evdns_base* GetDNSBase() const { return dns_base; };
  static Module* LoadModule(const String& path, const String& modName, String& retMsg);
protected:
  struct event_base* event_base;
  struct evdns_base* dns_base;
private:
  mutable set<Socket*> sockets;
  mutable set<Timer*> timers;
  void AddSocket(Socket* socket) const;
  void DelSocket(Socket* socket) const;
  void AddTimer(Timer* timer) const;
  void DelTimer(Timer* timer) const;
  const ModHandle so;
  const String modName;
  friend class Global;
  friend class Socket;
  friend class Timer;
};

};

#endif //_MODULE_H
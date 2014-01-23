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

#include <vector>
#include <event2/dns.h>
#include <event2/event.h>

#include "String.h"
#include "Module.h"

namespace trippingcyril {

class Global {
public:
  static Global* Get() {
    static Global* singleton = new Global;
    return singleton;
  };
  bool LoadModule(const String& path, String& retMsg);
  bool UnloadModule(const String& modName, String& retMsg);
  Module* FindModule(const String& module);
  InterModuleData ModuleInternalApiCall(const String& module, int method, void* arg = NULL);
  size_t LoadedModules() const { return modules.size(); };
  struct event_base* GetEventBase() const { return event_base; };
  struct evdns_base* GetDNSBase() const { return dns_base; };
  void Loop();
private:
  Global();
  virtual ~Global();

  std::vector<Module*> modules;
  struct event_base* event_base;
  struct evdns_base* dns_base;
};

};

#endif //_GLOBAL_H
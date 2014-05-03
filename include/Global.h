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

/**
 * @brief Our global class which has some key features
 */
class Global {
public:
  /** @return We're a singleton, so use Get() to well get me */
  static Global* Get() {
    static Global* singleton = new Global;
    return singleton;
  };
  /**
   * @brief Loads to module
   * @param path The path of the module to load
   * @param config Path to a config file
   * @return True if the module was succesfully loaded
   */
  bool LoadModule(const String& path, const String& config = "");
  /**
   * @brief Unloads a previously loaded module
   * @param modName The name of the module to unload
   * @param retMsg The return message
   * @return True if the module was succesfully unloaded
   */
  bool UnloadModule(const String& modName, String& retMsg);
  /**
   * @brief Get a loaded module using it's name
   * @param module The name of the module
   * @return The module if found, NULL otherwise
   */
  Module* FindModule(const String& module);
  /**
   * Makes a call to an internal module api
   * @param module What module's internal api do we want to call
   * @param method What method of this internal api do we want to call? Depends on your implementation of course
   * @param arg Custom data, also depends on your implementation
   * @return The InterModuleData, which is basically a wrapper around void* where module can still clean it up
   */
  InterModuleData ModuleInternalApiCall(const String& module, int method, void* arg = NULL);
  /** @return The amount of loaded modules */
  size_t LoadedModules() const { return modules.size(); };
  /** @return Our global libevent base */
  struct event_base* GetEventBase() const { return event_base; };
  /** @return Our global evdns base */
  struct evdns_base* GetDNSBase() const { return dns_base; };
  /** @brief Main application loop */
  void Loop();
  std::vector<Module*>::const_iterator begin() const { return modules.begin(); };
  std::vector<Module*>::const_iterator end() const { return modules.end(); };
private:
  // @cond
  Global();
  virtual ~Global();

  std::vector<Module*> modules;
  struct event_base* event_base;
  struct evdns_base* dns_base;
  // @endcond
};

};

#endif //_GLOBAL_H
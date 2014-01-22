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

/**
 * @brief Base class for modules
 */
class Module {
public:
  /**
   * Use MODCONSTRUCTOR and MODCONSTRUCTORHEADER instead of directly calling this
   * constructor yourself
   * @see MODCONSTRUCTOR
   * @see MODCONSTRUCTORHEADER
   */
  Module(ModHandle so, const String& modName);
  /**
   * Deconstructor, clean up your own resources here. Don't clean up any timers
   * or sockets that you allocated with this module, those will get cleaned up
   * for you. This includes any callbacks (if shouldDelete() returns true that is)
   */
  virtual ~Module();
  /**
   * Should return the version of the module, required to override
   */
  virtual String GetVersion() const = 0;
  /**
   * Will get called <b>once</b> after the module is loaded, initialize any timers
   * or sockets in this method, don't do that in your constructor!
   */
  virtual void OnLoaded() {};
  /**
   * Will get called for any calls to Global::ModuleInternalApiCall, implement
   * this method to actually create internal api methods.
   */
  virtual void* InternalApiCall(int method, void* arg) { return NULL; };
  /** @return The name of the module */
  const String GetModName() const { return modName; };
  /** @return The libevent base for this module */
  struct event_base* GetEventBase() const { return event_base; };
  /** @return The libevent dns base for this module */
  struct evdns_base* GetDNSBase() const { return dns_base; };
  /**
   * Used to actually load modules from shared libraries.
   * @param path The path of the shared library to actually load
   * @param modName The name the module should get, see Module::GetModName
   * @param retMsg A human readable string about the status of the loading
   * @return NULL on error, the loaded module otherwise
   */
  static Module* LoadModule(const String& path, const String& modName, String& retMsg);
  // @cond
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
  // @endcond
};

};

#endif //_MODULE_H
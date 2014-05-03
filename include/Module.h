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
#include <libconfig.h++>
#include <set>

#include "String.h"
#include "Event.h"

#define MODCONSTRUCTOR(CLASS) \
  CLASS::CLASS(ModHandle so, const String& modName, const String& path, const libconfig::Config* config) \
  : trippingcyril::Module(so, modName, path, config)

#define MODINLINECONSTRUCTOR(CLASS) \
  CLASS(ModHandle so, const String& modName, const String& path, const libconfig::Config* config) \
  : trippingcyril::Module(so, modName, path, config)

#define MODCONSTRUCTORHEADER(CLASS) \
  CLASS(ModHandle so, const String& modName, const String& path, const libconfig::Config* config)

#define MODULEDEFS(CLASS) \
  extern "C" { \
    Module* ModLoad(ModHandle so, const String& modName, const String& path, const libconfig::Config* config) { \
      return new CLASS(so, modName, path, config); \
    }; \
  };

namespace trippingcyril {

typedef void* ModHandle;

// @cond
class ModuleThread;
namespace thread {
  class Thread;
};
namespace net {
  class Socket;
};
namespace timing {
  class Timer;
};
// @endcond

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
  Module(ModHandle so, const String& modName, const String& path, const libconfig::Config* config);
  /**
   * Deconstructor, clean up your own resources here. Don't clean up any timers
   * or sockets that you allocated with this module, those will get cleaned up
   * for you. This includes any callbacks (if ShouldDelete::shouldDelete() returns true that is)
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
  /**
   * Used to clean up the data returned by InternalApiCall
   */
  virtual void CleanUpInterData(int method, void* arg) {};
  /** @return The name of the module */
  const String GetModName() const { return modName; };
  /** @return The path of the module */
  const String GetPath() const { return path; };
  /** @return The libevent base for this module */
  struct event_base* GetEventBase() const { return event_base; };
  /** @return The libevent dns base for this module */
  struct evdns_base* GetDNSBase() const { return dns_base; };
  /**
   * Used to actually load modules from shared libraries.
   * @param path The path of the shared library to actually load
   * @param modName The name the module should get, see Module::GetModName
   * @param retMsg A human readable string about the status of the loading
   * @param config Path to the configuration file of this module
   * @return NULL on error, the loaded module otherwise
   */
  static Module* LoadModule(const String& path, const String& modName, String& retMsg, const String& config);
protected:
  /** Set this to true in your constructor if you want to run in your own seperate thread */
  bool wantsThread : 1;
  /** Attempt to unload the module if it causes a crash, only works if wantsThread is true */
  bool unloadOnCrash : 1;
  /** Reload the module again after unloading it because of a crash, only works if unloadOnCrash is true */
  bool reloadOnCrash : 1;
  /** The loaded config file */
  const libconfig::Config *config;
  // @cond
  struct event_base* event_base;
  struct evdns_base* dns_base;
private:
  mutable set<const Event*> events;
  void AddEvent(const Event* event) const;
  void DelEvent(const Event* event) const;
  const ModHandle so;
  const String modName;
  const String path;
  friend class Global;
  friend class SubModuleLoader;
  friend class Event;

  thread::Thread* modThread;
  friend class ModuleThread;
  // @endcond
};

/**
 * @brief Abstraction layer around the void pointer coming from InternalApiCall
 * to allow for automatic destruction later on using CleanUpInterData
 */
class InterModuleData {
public:
  virtual ~InterModuleData() {
    if (module != NULL && data != NULL)
      module->CleanUpInterData(method, data);
  };
  /** @return True if an error happend, error as in module not found */
  bool hasError() { return module == NULL; };
  /** @return The actual custom data returned from InternalApiCall */
  void* GetData() { return data; };
private:
  // @cond
  InterModuleData(void* pData, Module* pModule, int pMethod) {
    this->module = pModule;
    this->data = pData;
    this->method = pMethod;
  };
  InterModuleData() {
    this->module = NULL;
    this->data = NULL;
  };
  void* data;
  int method;
  Module* module;
  friend class Global;
  // @endcond
};

};

#endif //_MODULE_H
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

#include "Module.h"

#include <dlfcn.h>

#include "Thread.h"
#include "Global.h"
#include "Socket.h"
#include "Timer.h"

namespace trippingcyril {

Module::Module(ModHandle pSo, const String& pModName, const String& pPath, const libconfig::Config* pConfig)
: config(pConfig)
, so(pSo)
, modName(pModName)
, path(pPath) {
  modThread = NULL;
  wantsThread = false;
  unloadOnCrash = false;
  reloadOnCrash = false;
  event_base = Global::Get()->GetEventBase();
  dns_base = Global::Get()->GetDNSBase();
};

Module::~Module() {
  if (modThread != NULL)
    delete modThread;
  while (events.empty() == false)
    delete *events.begin();
  if (event_base != Global::Get()->GetEventBase())
    event_base_free(event_base);
  if (config)
    delete config;
};

Module* Module::LoadModule(const String& path, const String& modName, String& retMsg, const String& configFile) {
  retMsg = "";
  ModHandle so = dlopen(path.c_str(), RTLD_NOW|RTLD_GLOBAL);
  if (so == NULL) {
    retMsg = "Unable to open module [" + modName + "] [" + dlerror() + "]";
    return NULL;
  };
  typedef Module* (fp)(ModHandle so, const String& modName, const String& path, const libconfig::Config* config);
  fp* ModLoad = (fp*) dlsym(so, "ModLoad");
  if (ModLoad == NULL) {
    dlclose(so);
    retMsg = "Could not find ModLoad() in module [" + modName + "]";
    return NULL;
  };
  libconfig::Config* config = NULL;
  if (configFile.empty() == false) {
    config = new libconfig::Config();
    try {
      config->readFile(configFile.c_str());
    } catch (const std::exception& error) {
      delete config;
      config = NULL;
    };
  };
  Module* output = ModLoad(so, modName, path, config);
  if (output == NULL) {
    retMsg = "ModLoad() returned NULL in module [" + modName + "]";
    dlclose(so);
    return NULL;
  };
  retMsg = "Loaded module [" + modName + "] [" + path + "]";
  return output;
};

void Module::AddEvent(const Event* event) const {
  events.insert(event);
};

void Module::DelEvent(const Event* event) const {
  for (set<const Event*>::iterator iter = events.begin(); iter != events.end(); ++iter) {
    if (*iter == event) {
      events.erase(iter);
      break;
    };
  };
};

};
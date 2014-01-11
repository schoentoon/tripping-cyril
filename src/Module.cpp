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

#include "Global.h"

namespace trippingcyril {

Module::Module(ModHandle so, const String& modName) {
  this->so = so;
  this->modName = modName;
  event_base = Global::Get()->GetEventBase();
  dns_base = Global::Get()->GetDNSBase();
};

Module::~Module() {
  if (event_base != Global::Get()->GetEventBase())
    event_base_free(event_base);
};

Module* Module::LoadModule(const String& path, const String& modName, String& retMsg) {
  retMsg = "";
  ModHandle so = dlopen(path.c_str(), RTLD_NOW|RTLD_GLOBAL);
  if (so == NULL) {
    retMsg = "Unable to open module [" + modName + "] [" + dlerror() + "]";
    return NULL;
  };
  typedef Module* (fp)(ModHandle so, const String& modName);
  fp* ModLoad = (fp*) dlsym(so, "ModLoad");
  if (ModLoad == NULL) {
    dlclose(so);
    retMsg = "Could not find ModLoad() in module [" + modName + "]";
    return NULL;
  };
  Module* output = ModLoad(so, modName);
  retMsg = "Loaded module [" + modName + "] [" + path + "]";
  return output;
};

};
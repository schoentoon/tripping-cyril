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

#include "Global.h"

#include <dlfcn.h>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "Files.h"
#include "Module.h"

namespace trippingcyril {

Global::Global() {
  SSL_library_init();
  ERR_load_CRYPTO_strings();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
  RAND_poll();
  event_base = event_base_new();
  dns_base = evdns_base_new(event_base, 1);
};

Global::~Global() {
  event_base_free(event_base);
  evdns_base_free(dns_base, 0);
};

void Global::Loop() {
  while (true)
    event_base_dispatch(event_base);
};

bool Global::LoadModule(const String& path, String& retMsg) {
  File f(path);
  for (unsigned int i = 0; i < modules.size(); i++) {
    if (modules[i]->GetModName() == f.GetShortName()) {
      retMsg = "Module [" + f.GetShortName() + "] is already loaded";
      return false;
    };
  };
  String modname = f.GetShortName();
  String::size_type sPos = modname.rfind('.');
  if (sPos != String::npos)
    modname = modname.substr(0, sPos);
  Module* module = Module::LoadModule(path, modname, retMsg);
  if (module == NULL) {
    std::cerr << retMsg << std::endl;
    return false;
  };
  modules.push_back(module);
  module->OnLoaded();
  return true;
};

bool Global::UnloadModule(const String& modName, String& retMsg) {
  vector<Module*>::iterator iter;
  for (iter = modules.begin(); iter != modules.end(); ++iter) {
    if ((*iter)->GetModName() == modName) {
      break;
    };
  };
  if (iter == modules.end()) {
    retMsg = "Module [" + modName + "] isn't loaded?";
    return false;
  };
  ModHandle so = (*iter)->so;
  if (so) {
    delete *iter;
    modules.erase(iter);
    dlclose(so);
    retMsg = "Module [" + modName + "] unloaded";
    return true;
  };
  retMsg = "Unable to unload module [" + modName + "]";
  return false;
};

};
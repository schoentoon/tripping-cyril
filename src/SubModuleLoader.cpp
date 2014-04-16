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

#include "SubModuleLoader.h"

#include <dlfcn.h>
#include <iostream>

#include "Files.h"
#include "Module.h"
#include "TermUtils.h"
#include "ModuleThread.h"

namespace trippingcyril {

SubModuleLoader::SubModuleLoader() {
};

SubModuleLoader::~SubModuleLoader() {
  String msg;
  while (modules.empty() == false) {
    bool ret = UnloadModule((*modules.begin())->GetModName(), msg);
    TermUtils::PrintStatus(ret, msg);
  };
};

bool SubModuleLoader::LoadModule(const String& path) {
  TermUtils::StatusPrinter status("Loading [" + path + "]");
  File f(path);
  for (unsigned int i = 0; i < modules.size(); i++) {
    if (modules[i]->GetModName() == f.GetShortName()) {
      status.PrintStatus(false, "Module [" + f.GetShortName() + "] is already loaded");
      return false;
    };
  };
  String modname = f.GetShortName();
  String::size_type sPos = modname.rfind('.');
  if (sPos != String::npos)
    modname = modname.substr(0, sPos);
  String retMsg;
  Module* module = Module::LoadModule(path, modname, retMsg);
  if (module == NULL) {
    status.PrintStatus(false, retMsg);
    return false;
  };
  if (validateModule(module, retMsg)) {
    status.PrintStatus(true, retMsg);
    modules.push_back(module);
    if (module->wantsThread) {
      ModuleThread* thread = new ModuleThread(module);
      thread->Start();
    } else
      module->OnLoaded();
  } else {
    status.PrintStatus(false, retMsg);
    delete module;
    return false;
  };
  return true;
};

bool SubModuleLoader::UnloadModule(const String& modName, String& retMsg) {
  vector<Module*>::iterator iter;
  for (iter = modules.begin(); iter != modules.end(); ++iter) {
    if ((*iter)->GetModName() == modName)
      break;
  };
  if (iter == modules.end()) {
    retMsg = "Module [" + modName + "] isn't loaded?";
    return false;
  };
  ModHandle so = (*iter)->so;
  if (so != NULL) {
    delete *iter;
    modules.erase(iter);
    if (dlclose(so) == 0) {
      retMsg = "Module [" + modName + "] unloaded";
      return true;
    } else {
      retMsg = "Unable to unload module [" + modName + "] [" + dlerror() + "]";
      return false;
    };
  };
  retMsg = "Unable to unload module [" + modName + "]";
  return false;
};

Module* SubModuleLoader::FindModule(const String& module) {
  vector<Module*>::const_iterator iter;
  for (iter = modules.begin(); iter != modules.end(); ++iter) {
    if ((*iter)->GetModName() == module)
      return *iter;
  };
  return NULL;
};

};
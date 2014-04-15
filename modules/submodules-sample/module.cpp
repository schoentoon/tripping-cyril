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

#include "module.h"

#include "submodule.h"

#include "Timer.h"

#include <iostream>

using namespace trippingcyril::timing;

using namespace module::submodules;

class TickTimer : public Timer {
public:
  TickTimer(const Module* module, SubModuleLoader* modules)
  : Timer(module, 10) {
    this->modules = modules;
  };
  virtual ~TickTimer() {
  };
protected:
  virtual void RunJob() {
    for (std::vector<Module*>::const_iterator iter = modules->begin(); iter != modules->end(); ++iter) {
      SubModule* submod = dynamic_cast<SubModule*>(*iter);
      submod->onTick();
    };
  };
private:
  SubModuleLoader* modules;
};

class TickingSubModuleLoader : public SubModuleLoader {
public:
  TickingSubModuleLoader() : SubModuleLoader() {};
  virtual ~TickingSubModuleLoader() {};
protected:
  virtual bool validateModule(const Module* module, String& retMsg) {
    return dynamic_cast<const SubModule*>(module) != NULL;
  };
};

MODCONSTRUCTOR(SampleModule) {
  modules = new TickingSubModuleLoader;
};

SampleModule::~SampleModule() {
  delete modules;
};

void SampleModule::OnLoaded() {
  modules->LoadModule("modules/submodules-sample/submodule_a.so");
  modules->LoadModule("modules/submodules-sample/submodule_b.so");
  new TickTimer(this, modules);
};

MODULEDEFS(SampleModule);
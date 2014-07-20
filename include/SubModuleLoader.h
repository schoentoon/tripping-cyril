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

#pragma once

#include <vector>

#include "String.h"
#include "Module.h"

namespace trippingcyril {

/**
 * @brief Our submodule loader, use this to load modules from within modules (we need to go deeper)
 */
class SubModuleLoader {
public:
  SubModuleLoader();
  virtual ~SubModuleLoader();
  /**
   * @brief Loads to module
   * @param path The path of the module to load
   * @param config Path to config file
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
  /** @return The amount of loaded modules */
  size_t LoadedModules() const { return modules.size(); };
  std::vector<Module*>::const_iterator begin() const { return modules.begin(); };
  std::vector<Module*>::const_iterator end() const { return modules.end(); };
protected:
  /**
   * Override this to validateModules, as you'll want to make sure that all your
   * modules are the same.
   * @param module The module to validate
   * @param retMsg If you refuse it, it would be nice to fill in why in this parameter
   * @return True if valid, false otherwise. It'll automatically get unloaded then.
   */
  virtual bool validateModule(const Module* module, String& retMsg) = 0;
private:
  // @cond
  std::vector<Module*> modules;
  // @endcond
};

};
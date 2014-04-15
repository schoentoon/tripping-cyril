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

#include "submodule.h"

#include <iostream>

namespace module {
  namespace submodules {

class SubModuleB : public SubModule {
public:
  SubModuleB(ModHandle so, const String& modName, const String& path)
  : SubModule(so, modName, path) {
  };
  virtual ~SubModuleB() {
  };
  virtual void onTick() const {
    std::cout << "SubModuleB::onTick();" << std::endl;
  };
};

  };
};

MODULEDEFS(module::submodules::SubModuleB);
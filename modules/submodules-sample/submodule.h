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

#ifndef _SUB_MOD_H
#define _SUB_MOD_H

#include "Module.h"

using namespace trippingcyril;

namespace module {
  namespace submodules {

class SubModule : public Module {
public:
  MODINLINECONSTRUCTOR(SubModule) {};
  virtual ~SubModule() {};
  String GetVersion() const { return "sample"; };
  virtual void onTick() const = 0;
};

  };
};

#endif //_SUB_MOD_H
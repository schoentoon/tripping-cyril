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

#include <gtest/gtest.h>

#include "Module.h"
#include "Global.h"

using namespace trippingcyril;

namespace test {

TEST(Module, DISABLED_LoadDummy) { //TODO Figure out why this sometimes failed :/
  String msg;
  EXPECT_EQ(0, Global::Get()->LoadedModules());
  ASSERT_TRUE(Global::Get()->LoadModule("./dummymod.so"));
  EXPECT_EQ(msg, "Loaded module [dummymod] [./dummymod.so]");
  EXPECT_EQ(1, Global::Get()->LoadedModules());
  ASSERT_TRUE(Global::Get()->UnloadModule("dummymod", msg));
  EXPECT_EQ(msg, "Module [dummymod] unloaded");
  EXPECT_EQ(0, Global::Get()->LoadedModules());
};

};
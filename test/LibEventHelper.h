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

#ifndef _LIBEVENT_HELPER_H
#define _LIBEVENT_HELPER_H

#include <gtest/gtest.h>

#include "Module.h"

using namespace trippingcyril;

namespace test {

class LibEventHelper : public Module {
public:
  LibEventHelper()
  : Module(NULL, "libevent") {
    event_base = event_base_new();
    struct timeval tm;
    tm.tv_usec = 0;
    tm.tv_sec = 10;
    EXPECT_EQ(0, event_base_loopexit(event_base, &tm));
  };
  String GetVersion() { return ""; };
  bool Loop() {
    return event_base_dispatch(event_base) == 0;
  };
};

};

#endif //_LIBEVENT_HELPER_H
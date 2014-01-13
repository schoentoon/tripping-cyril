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

#include "TermUtils.h"

#include <stdio.h>

namespace trippingcyril {

void TermUtils::PrintStatus(bool status, const String& pMsg) {
  String msg(pMsg);
  msg.Trim();
  if (msg.empty() == false) {
    if (status)
      fprintf(stdout, "\033[1m\033[34m[\033[32m ok \033[34m]\033[39m\033[22m %s\n", msg.c_str());
    else
      fprintf(stdout, "\033[1m\033[34m[\033[31m !! \033[34m]\033[39m\033[22m %s\n", msg.c_str());
    fflush(stdout);
  } else
    fprintf(stderr, "Uh?...\n");
};

};
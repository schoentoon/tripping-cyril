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

#include "Pipe.h"

#include <string.h>
#include <errno.h>

#include "Global.h"

namespace trippingcyril {

Pipe::Pipe(const Module* pModule)
: module(pModule) {
  if (pipe(fds) != 0)
    throw String(strerror(errno));
  fcntl(fds[0], F_SETFL, O_NONBLOCK);
  read_event = event_new(module != NULL ? module->GetEventBase() : Global::Get()->GetEventBase()
                        ,fds[0], EV_PERSIST|EV_READ, Pipe::EventCallback, this);
  event_add(read_event, NULL);
};

Pipe::~Pipe() {
  close(fds[0]);
  close(fds[1]);
  event_free(read_event);
};

void Pipe::EventCallback(evutil_socket_t fd, short event, void* arg) {
  Pipe* pipe = (Pipe*) arg;
  pipe->OnRead();
};

};
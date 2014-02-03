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

#ifndef _POSTGRES_H
#define _POSTGRES_H

#include <libpq-fe.h>
#include <event2/event.h>
#include <deque>

#include "Database.h"

namespace trippingcyril {

class PQJob;

class NonBlockingPostGres : public Database {
public:
  NonBlockingPostGres(const String& connstring, const Module *pModule = NULL);
  virtual ~NonBlockingPostGres();
  virtual const DBResult* Select(const String& query, DBCallback *callback = NULL);
  virtual const DBResult* Insert(const String& query, DBCallback *callback = NULL);
  bool isIdle() const { return conn == NULL; };
private:
  String connstring;
  deque<PQJob*> jobs;
  PGconn *conn;
  struct event *event;
  bool in_loop : 1;
  static void EventCallback(evutil_socket_t fd, short event, void* ctx);
  void Loop();
};

};

#endif // _POSTGRES_H
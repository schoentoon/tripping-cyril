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

// @cond
class PQJob;
// @endcond

/**
 * @brief A non blocking postgresql implementation of the Database interface
 */
class NonBlockingPostGres : public Database {
public:
  /**
   * General constructor
   * @param connstring The connection string for this database
   * @param pModule The module to register this database on
   */
  NonBlockingPostGres(const String& connstring, const Module *pModule = NULL);
  /** General deconstructor
   */
  virtual ~NonBlockingPostGres();
  virtual const DBResult* Select(const String& query, DBCallback *callback = NULL);
  virtual const DBResult* Insert(const String& query, DBCallback *callback = NULL);
  bool isIdle() const { return conn == NULL; };
private:
  // @cond
  String connstring;
  deque<PQJob*> jobs;
  PGconn *conn;
  struct event *event;
  bool in_loop : 1;
  static void EventCallback(evutil_socket_t fd, short event, void* ctx);
  void Loop();
  // @endcond
};

class PostGres : public Database {
public:
  /**
   * General constructor
   * @param connstring The connection string for this database
   * @param pModule The module to register this database on
   */
  PostGres(const String& connstring, const Module* pModule = NULL);
  /** General deconstructor
   */
  virtual ~PostGres();
  virtual const DBResult* Select(const String& query, DBCallback *callback = NULL);
  virtual const DBResult* Insert(const String& query, DBCallback *callback = NULL);
  bool isIdle() const { return true; };
private:
  // @cond
  void Connect();
  String connstring;
  PGconn *conn;
  // @endcond
};

};

#endif // _POSTGRES_H
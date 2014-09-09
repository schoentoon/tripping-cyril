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

#ifndef _NO_POSTGRES

#ifndef _PG_BACKOFF_START
#  define _PG_BACKOFF_START 5.0
#endif
#ifndef _PG_BACKOFF_STEP
#  define _PG_BACKOFF_STEP 5.0
#endif
#ifndef _PG_BACKOFF_MAX
#  define _PG_BACKOFF_MAX 15.0
#endif

#include <libpq-fe.h>
#include <event2/event.h>
#include <deque>
#include <map>

#include "defines.h"
#include "Database.h"
#include "BackoffTimer.h"

namespace trippingcyril {
  namespace database {
    namespace postgres {

// @cond
struct PQJob;
class PostGres;
class PostGresBackoff;
// @endcond

/**
 * @brief Callback for listen/notify operations within PostGres, use this with PostGres::Listen
 */
class PGNotifyListener : public ShouldDelete {
public:
  virtual ~PGNotifyListener() {
  };
protected:
  /**
   * Will get called upon a notify operation
   * @param key The key, or channel as they call it within postgres
   * @param payload The optional payload
   * @param pid The process id of the postgresql backend
   */
  virtual void onNotify(const String& key, const String& payload, const int pid) = 0;
  // @cond
  friend class PostGres;
  // @endcond
};

/**
 * @brief A non blocking postgresql implementation of the Database interface
 * @note This implementation is somewhat threadsafe, all queries will be executed on
 * the thread that the event base is running on, however the query queueing methods
 * are not threadsafe!
 */
class PostGres : public Database {
public:
  /**
   * General constructor
   * @param connstring The connection string for this database
   * @param pModule The module to register this database on
   */
  PostGres(const String& connstring, const Module *pModule = NULL);
  /** General deconstructor
   */
  virtual ~PostGres();
#if __cplusplus >= 201103
  void SelectLamdba(const String& query, const DBLamdbaCallback &callback
  , const DBLamdbaErrorCallback &errorcallback = nullptr) OVERRIDE;
  void InsertLamdba(const String& query, const DBLamdbaCallback &callback
  , const DBLamdbaErrorCallback &errorcallback = nullptr) OVERRIDE;
#endif
   const DBResult* Select(const String& query, DBCallback *callback = NULL) OVERRIDE;
  const DBResult* Insert(const String& query, DBCallback *callback = NULL) OVERRIDE;
  bool isIdle() const OVERRIDE { return _conn == NULL; };
  /**
   * Demands SetStayConnected(true), will apply automatically
   * @return False if you're already listening for this or if it contained some
   * invalid characters @see String::OnlyContains
   */
  bool Listen(const String& key, PGNotifyListener* listener);
  void Unlisten(const String& key);
  virtual void SetStayConnected(bool b) OVERRIDE {
    if (_listeners.empty())
      _stay_connected = b;
  };
private:
  // @cond
  String _connstring;
  deque<PQJob*> _jobs;
  PGconn *_conn;
  struct event *_event;
  bool _in_loop : 1;
  std::map<String, PGNotifyListener*> _listeners;
  timing::BackoffTimer* _backoff;

  static void EventCallback(evutil_socket_t fd, short event, void* ctx);
  void Loop();
  void Connect();
  friend class PostGresBackoff;
  // @endcond
};

/**
 * @brief A blocking postgresql implementation of the Database interface
 * @note This implementation is NOT threadsafe, all calls to it should be made
 * on the same thread.
 */
class BlockingPostGres : public Database {
public:
  /**
   * General constructor
   * @param connstring The connection string for this database
   * @param pModule The module to register this database on
   */
  BlockingPostGres(const String& connstring, const Module* pModule = NULL);
  /** General deconstructor
   */
  virtual ~BlockingPostGres();
  const DBResult* Select(const String& query, DBCallback *callback = NULL) OVERRIDE;
  const DBResult* Insert(const String& query, DBCallback *callback = NULL) OVERRIDE;
  bool isIdle() const OVERRIDE { return true; };
private:
  // @cond
  void Connect();
  String _connstring;
  PGconn *_conn;
  // @endcond
};

    };
  };
};

#endif //_NO_POSTGRES
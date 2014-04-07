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

#include "Postgres.h"

#ifndef _NO_POSTGRES

#include "Global.h"
#include "TermUtils.h"

namespace trippingcyril {

class PQJob {
public:
  PQJob(const String &pQuery)
  : query(pQuery) {
    callback = NULL;
    sent = false;
#if __cplusplus >= 201103
    lamdba_callback = NULL;
    lamdba_errorcallback = NULL;
#endif
  }
  virtual ~PQJob() {
    if (callback != NULL && callback->shouldDelete())
      delete callback;
  }
private:
  const String query;
  DBCallback* callback;
  bool sent : 1;
#if __cplusplus >= 201103
  const DBLamdbaCallback *lamdba_callback;
  const DBLamdbaErrorCallback *lamdba_errorcallback;
#endif
  friend class PostGres;
};

class DBPGResult : public DBResult {
public:
  DBPGResult(PGresult* result) {
    this->result = result;
  };
  virtual ~DBPGResult() {
    PQclear(result);
  };
  const int columns() const {
    return PQnfields(result);
  };
  const int rows() const {
    return PQntuples(result);
  };
  const String getValue(int row, int column) const {
    return String(PQgetvalue(result, row, column));
  };
  const bool hasError() const {
    ExecStatusType status = PQresultStatus(result);
    return status != PGRES_COMMAND_OK;
  };
  const String getError() const {
    String error(PQresultErrorMessage(result));
    error.Trim();
    return error;
  };
  const bool isNull(int row, int column) const {
    return PQgetisnull(result, row, column);
  };
private:
  PGresult* result;
};

class PostGresBackoff : public BackoffTimer {
public:
  PostGresBackoff(PostGres* pPg)
  : BackoffTimer(pPg->GetModule(), _PG_BACKOFF_START, _PG_BACKOFF_STEP, _PG_BACKOFF_MAX) {
    pg = pPg;
  };
  virtual ~PostGresBackoff() {
  };
protected:
  virtual void RunJob() {
    pg->Connect();
  };
private:
  PostGres* pg;
};

PostGres::PostGres(const String& connstring, const Module* pModule)
: Database(pModule) {
  conn = NULL;
  event = NULL;
  backoff = NULL;
  in_loop = false;
  this->connstring = connstring;
};

PostGres::~PostGres() {
  if (conn)
    PQfinish(conn);
  if (event)
    event_free(event);
  if (backoff)
    delete backoff;
  while (!jobs.empty()) {
    delete jobs.front();
    jobs.pop_front();
  };
  while (!listeners.empty()) {
    std::map<String, PGNotifyListener*>::iterator iter = listeners.begin();
    if (iter->second->shouldDelete())
      delete iter->second;
    listeners.erase(iter);
  };
};

#if __cplusplus >= 201103
void PostGres::SelectLamdba(const String& query, const DBLamdbaCallback& callback, const DBLamdbaErrorCallback& errorcallback) {
  PQJob *job = new PQJob(query);
  job->lamdba_callback = &callback;
  job->lamdba_errorcallback = &errorcallback;
  jobs.push_back(job);
  Loop();
};

void PostGres::InsertLamdba(const String& query, const DBLamdbaCallback& callback, const DBLamdbaErrorCallback& errorcallback) {
  SelectLamdba(query, callback, errorcallback);
};
#endif

const DBResult* PostGres::Select(const String& query, DBCallback* callback) {
  PQJob *job = new PQJob(query);
  job->callback = callback;
  jobs.push_back(job);
  Loop();
  return NULL;
};

const DBResult* PostGres::Insert(const String& query, DBCallback* callback) {
  return Select(query, callback);
};

bool PostGres::Listen(const String& key, PGNotifyListener* listener) {
  if (key.OnlyContains("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-") == false)
    return false;
  std::map<String, PGNotifyListener*>::iterator iter = listeners.find(key);
  if (iter == listeners.end()) {
    listeners[key] = listener;
    stay_connected = true;
    Select("LISTEN \"" + key + "\"");
    return true;
  };
  return false;
};

void PostGres::Unlisten(const String& key) {
  std::map<String, PGNotifyListener*>::iterator iter = listeners.find(key);
  if (iter != listeners.end()) {
    if (iter->second->shouldDelete())
      delete iter->second;
    listeners.erase(iter);
    Select("UNLISTEN \"" + key + "\"");
  };
};

void PostGres::EventCallback(int fd, short int event, void* ctx) {
  PostGres* pg = (PostGres*) ctx;
  pg->Loop();
};

void PostGres::Loop() {
  if (in_loop)
    return;
  if (conn != NULL) {
    if (PQconsumeInput(conn) != 1) {
      if (PQstatus(conn) != CONNECTION_OK && event != NULL) {
        event_free(event);
        PQfinish(conn);
        conn = NULL;
        event = NULL;
        if (stay_connected || jobs.empty() == false) {
          if (backoff == NULL)
            backoff = new PostGresBackoff(this);
        };
      };
    };
    if (PQisBusy(conn) == 0) {
      PGnotify* notify;
      while ((notify = PQnotifies(conn)) != NULL) {
        std::map<String, PGNotifyListener*>::const_iterator iter = listeners.find(notify->relname);
        if (iter != listeners.end() && iter->second != NULL)
          iter->second->onNotify(notify->relname, notify->extra, notify->be_pid);
        PQfreemem(notify);
      };
      if (jobs.empty() == false) {
        PQJob* job = jobs[0];
        if (!job->sent) {
          PQsendQuery(conn, job->query.c_str());
          job->sent = true;
        };
        in_loop = true;
        PGresult *result = PQgetResult(conn);
        while (result != NULL) {
          DBPGResult dbresult(result);
          if (dbresult.hasError()) {
            if (job->callback != NULL)
              job->callback->QueryError(dbresult.getError(), job->query);
#if __cplusplus >= 201103
            if (job->lamdba_errorcallback != NULL)
              (*job->lamdba_errorcallback)(dbresult.getError(), job->query);
#endif
          } else {
            if (job->callback != NULL)
              job->callback->QueryResult(&dbresult, job->query);
#if __cplusplus >= 201103
            if (job->lamdba_callback != NULL)
              (*job->lamdba_callback)(&dbresult, job->query);
#endif
          };
          result = PQgetResult(conn);
        };
        if (job->callback != NULL)
          job->callback->QueryDone();
        jobs.pop_front();
        delete job;
        in_loop = false;
        Loop();
      };
    } else if (jobs.empty() && listeners.empty() && stay_connected == false) {
      if (event != NULL) {
        event_free(event);
        PQfinish(conn);
        conn = NULL;
        event = NULL;
      };
    };
  } else if (jobs.empty() == false && conn == NULL)
    Connect();
};

void PostGres::Connect() {
  if (conn == NULL) {
    conn = PQconnectdb(connstring.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
      PQfinish(conn);
      conn = NULL;
      if (backoff != NULL)
        backoff->SetStillFailing();
      else if (stay_connected || jobs.empty() == false)
        backoff = new PostGresBackoff(this);
    } else {
      if (backoff != NULL) {
        delete backoff;
        backoff = NULL;
      };
      PQsetnonblocking(conn, 1);
      if (autocommit) {
        PQJob* autocommit_job = new PQJob("SET AUTOCOMMIT = ON");
        jobs.push_front(autocommit_job);
      };
      if (listeners.empty() == false) {
        for (std::map<String, PGNotifyListener*>::const_iterator iter = listeners.begin(); iter != listeners.end(); ++iter) {
          PQJob* job = new PQJob("LISTEN \"" + iter->first + "\"");
          jobs.push_front(job);
        };
      };
      if (event == NULL) {
        event = event_new(GetEventBase(), PQsocket(conn), EV_READ|EV_PERSIST, PostGres::EventCallback, this);
        event_add(event, NULL);
      };
      Loop();
    };
  };
};

BlockingPostGres::BlockingPostGres(const String& connstring, const Module* pModule)
: Database(pModule) {
  conn = NULL;
  this->connstring = connstring;
};

BlockingPostGres::~BlockingPostGres() {
  if (conn != NULL)
    PQfinish(conn);
};

const DBResult* BlockingPostGres::Select(const String& query, DBCallback* callback) {
  Connect();
  PGresult* result = PQexec(conn, query.c_str());
  DBPGResult* dbresult = new DBPGResult(result);
  if (callback) {
    if (dbresult->hasError())
      callback->QueryError(dbresult->getError(), query);
    else
      callback->QueryResult(dbresult, query);
    if (callback->shouldDelete())
      delete callback;
  };
  return dbresult;
};

const DBResult* BlockingPostGres::Insert(const String& query, DBCallback* callback) {
  return Select(query, callback);
};

void BlockingPostGres::Connect() {
  while (conn == NULL) {
    conn = PQconnectdb(connstring.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
      TermUtils::PrintStatus(false, PQerrorMessage(conn));
      PQfinish(conn);
      conn = NULL;
    } else {
      if (autocommit) {
        PGresult* result = PQexec(conn, "SET AUTOCOMMIT = ON");
        DBPGResult dbresult(result);
      };
    };
  };
};

};

#endif //_NO_POSTGRES
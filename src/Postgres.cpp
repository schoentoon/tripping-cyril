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
  namespace database {
    namespace postgres {

struct PQJob {
public:
  PQJob(const String &pQuery)
  : _query(pQuery)
  , _callback(NULL)
  , _sent(false)
#if __cplusplus >= 201103
  , _lamdba_callback(nullptr)
  , _lamdba_errorcallback(nullptr)
#endif
    {
  }
  virtual ~PQJob() {
    if (_callback != NULL && _callback->shouldDelete())
      delete _callback;
  }
private:
  const String _query;
  DBCallback* _callback;
  bool _sent : 1;
#if __cplusplus >= 201103
  const DBLamdbaCallback *_lamdba_callback;
  const DBLamdbaErrorCallback *_lamdba_errorcallback;
#endif
  friend class PostGres;
};

struct DBPGResult : public DBResult {
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

class PostGresBackoff : public timing::BackoffTimer {
public:
  PostGresBackoff(PostGres* pPg)
  : timing::BackoffTimer(pPg->GetModule(), _PG_BACKOFF_START, _PG_BACKOFF_STEP, _PG_BACKOFF_MAX)
  , _pg(pPg) {
  };
  virtual ~PostGresBackoff() {
  };
protected:
  virtual void RunJob() {
    _pg->Connect();
  };
private:
  PostGres* _pg;
};

PostGres::PostGres(const String& connstring, const Module* pModule)
: Database(pModule)
, _connstring(connstring)
, _conn(NULL)
, _event(NULL)
, _in_loop(false)
, _backoff(NULL) {
};

PostGres::~PostGres() {
  if (_conn)
    PQfinish(_conn);
  if (_event)
    event_free(_event);
  if (_backoff)
    delete _backoff;
  while (!_jobs.empty()) {
    delete _jobs.front();
    _jobs.pop_front();
  };
  while (!_listeners.empty()) {
    std::map<String, PGNotifyListener*>::iterator iter = _listeners.begin();
    if (iter->second->shouldDelete())
      delete iter->second;
    _listeners.erase(iter);
  };
};

#if __cplusplus >= 201103
void PostGres::SelectLamdba(const String& query, const DBLamdbaCallback& callback, const DBLamdbaErrorCallback& errorcallback) {
  PQJob *job = new PQJob(query);
  job->_lamdba_callback = &callback;
  job->_lamdba_errorcallback = &errorcallback;
  _jobs.push_back(job);
  Loop();
};

void PostGres::InsertLamdba(const String& query, const DBLamdbaCallback& callback, const DBLamdbaErrorCallback& errorcallback) {
  SelectLamdba(query, callback, errorcallback);
};
#endif

const DBResult* PostGres::Select(const String& query, DBCallback* callback) {
  PQJob *job = new PQJob(query);
  job->_callback = callback;
  _jobs.push_back(job);
  Loop();
  return NULL;
};

const DBResult* PostGres::Insert(const String& query, DBCallback* callback) {
  return Select(query, callback);
};

bool PostGres::Listen(const String& key, PGNotifyListener* listener) {
  if (key.OnlyContains("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-") == false)
    return false;
  std::map<String, PGNotifyListener*>::iterator iter = _listeners.find(key);
  if (iter == _listeners.end()) {
    _listeners[key] = listener;
    _stay_connected = true;
    Select("LISTEN \"" + key + "\"");
    return true;
  };
  return false;
};

void PostGres::Unlisten(const String& key) {
  std::map<String, PGNotifyListener*>::iterator iter = _listeners.find(key);
  if (iter != _listeners.end()) {
    if (iter->second->shouldDelete())
      delete iter->second;
    _listeners.erase(iter);
    Select("UNLISTEN \"" + key + "\"");
  };
};

void PostGres::EventCallback(int fd, short int event, void* ctx) {
  PostGres* pg = (PostGres*) ctx;
  pg->Loop();
};

void PostGres::Loop() {
  if (_in_loop)
    return;
  if (_conn != NULL) {
    if (PQconsumeInput(_conn) != 1) {
      if (PQstatus(_conn) != CONNECTION_OK && _event != NULL) {
        event_free(_event);
        PQfinish(_conn);
        _conn = NULL;
        _event = NULL;
        if (_stay_connected || _jobs.empty() == false) {
          if (_backoff == NULL)
            _backoff = new PostGresBackoff(this);
        };
      };
    };
    if (PQisBusy(_conn) == 0) {
      PGnotify* notify;
      while ((notify = PQnotifies(_conn)) != NULL) {
        std::map<String, PGNotifyListener*>::const_iterator iter = _listeners.find(notify->relname);
        if (iter != _listeners.end() && iter->second != NULL)
          iter->second->onNotify(notify->relname, notify->extra, notify->be_pid);
        PQfreemem(notify);
      };
      if (_jobs.empty() == false) {
        PQJob* job = _jobs[0];
        if (!job->_sent) {
          PQsendQuery(_conn, job->_query.c_str());
          job->_sent = true;
        };
        _in_loop = true;
        PGresult *result = PQgetResult(_conn);
        while (result != NULL) {
          DBPGResult dbresult(result);
          if (dbresult.hasError()) {
            if (job->_callback != NULL)
              job->_callback->QueryError(dbresult.getError(), job->_query);
#if __cplusplus >= 201103
            if (job->_lamdba_errorcallback != NULL && *job->_lamdba_errorcallback)
              (*job->_lamdba_errorcallback)(dbresult.getError(), job->_query);
#endif
          } else {
            if (job->_callback != NULL)
              job->_callback->QueryResult(&dbresult, job->_query);
#if __cplusplus >= 201103
            if (job->_lamdba_callback != NULL && *job->_lamdba_callback)
              (*job->_lamdba_callback)(&dbresult, job->_query);
#endif
          };
          result = PQgetResult(_conn);
        };
        if (job->_callback != NULL)
          job->_callback->QueryDone();
        _jobs.pop_front();
        delete job;
        _in_loop = false;
        Loop();
      };
    } else if (_jobs.empty() && _listeners.empty() && _stay_connected == false) {
      if (_event != NULL) {
        event_free(_event);
        PQfinish(_conn);
        _conn = NULL;
        _event = NULL;
      };
    };
  } else if (_jobs.empty() == false && _conn == NULL)
    Connect();
};

void PostGres::Connect() {
  if (_conn == NULL) {
    _conn = PQconnectdb(_connstring.c_str());
    if (PQstatus(_conn) != CONNECTION_OK) {
      PQfinish(_conn);
      _conn = NULL;
      if (_backoff != NULL)
        _backoff->SetStillFailing();
      else if (_stay_connected || _jobs.empty() == false)
        _backoff = new PostGresBackoff(this);
    } else {
      if (_backoff != NULL) {
        delete _backoff;
        _backoff = NULL;
      };
      PQsetnonblocking(_conn, 1);
      if (_autocommit) {
        PQJob* autocommit_job = new PQJob("SET AUTOCOMMIT = ON");
        _jobs.push_front(autocommit_job);
      };
      if (_listeners.empty() == false) {
        for (std::map<String, PGNotifyListener*>::const_iterator iter = _listeners.begin(); iter != _listeners.end(); ++iter) {
          PQJob* job = new PQJob("LISTEN \"" + iter->first + "\"");
          _jobs.push_front(job);
        };
      };
      if (_event == NULL) {
        _event = event_new(GetEventBase(), PQsocket(_conn), EV_READ|EV_PERSIST, PostGres::EventCallback, this);
        event_add(_event, NULL);
      };
      Loop();
    };
  };
};

BlockingPostGres::BlockingPostGres(const String& connstring, const Module* pModule)
: Database(pModule)
, _connstring(connstring)
, _conn(NULL) {
};

BlockingPostGres::~BlockingPostGres() {
  if (_conn != NULL)
    PQfinish(_conn);
};

const DBResult* BlockingPostGres::Select(const String& query, DBCallback* callback) {
  Connect();
  PGresult* result = PQexec(_conn, query.c_str());
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
  while (_conn == NULL) {
    _conn = PQconnectdb(_connstring.c_str());
    if (PQstatus(_conn) != CONNECTION_OK) {
      TermUtils::PrintStatus(false, PQerrorMessage(_conn));
      PQfinish(_conn);
      _conn = NULL;
    } else {
      if (_autocommit) {
        PGresult* result = PQexec(_conn, "SET AUTOCOMMIT = ON");
        DBPGResult dbresult(result);
      };
    };
  };
};

    };
  };
};

#endif //_NO_POSTGRES
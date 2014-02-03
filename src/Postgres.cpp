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

#include "Global.h"
#include "TermUtils.h"

namespace trippingcyril {

class PQJob {
public:
  PQJob(const String &pQuery)
  : query(pQuery) {
    callback = NULL;
    sent = false;
  }
  virtual ~PQJob() {
    if (callback != NULL && callback->shouldDelete())
      delete callback;
  }
private:
  const String query;
  DBCallback* callback;
  bool sent : 1;
  friend class NonBlockingPostGres;
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
    return (status == PGRES_BAD_RESPONSE
        or status == PGRES_NONFATAL_ERROR
        or status == PGRES_FATAL_ERROR);
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

NonBlockingPostGres::NonBlockingPostGres(const String& connstring, const Module* pModule)
: Database(pModule) {
  conn = NULL;
  event = NULL;
  in_loop = false;
  this->connstring = connstring;
};

NonBlockingPostGres::~NonBlockingPostGres() {
  if (conn)
    PQfinish(conn);
  if (event)
    event_free(event);
  while (!jobs.empty()) {
    delete jobs.front();
    jobs.pop_front();
  };
};

const DBResult* NonBlockingPostGres::Select(const String& query, DBCallback* callback) {
  PQJob *job = new PQJob(query);
  job->callback = callback;
  jobs.push_back(job);
  Loop();
  return NULL;
};

const DBResult* NonBlockingPostGres::Insert(const String& query, DBCallback* callback) {
  return Select(query, callback);
};

void NonBlockingPostGres::EventCallback(int fd, short int event, void* ctx) {
  NonBlockingPostGres* pg = (NonBlockingPostGres*) ctx;
  pg->Loop();
};

void NonBlockingPostGres::Loop() {
  if (in_loop)
    return;
  if (jobs.empty()) {
    if (event && conn) {
      event_free(event);
      PQfinish(conn);
      conn = NULL;
      event = NULL;
    };
  } else if (conn != NULL && !jobs.empty()) {
    PQJob* job = jobs[0];
    if (!job->sent) {
      PQsendQuery(conn, job->query.c_str());
      job->sent = true;
    };
    if (PQconsumeInput(conn) != 1)
      TermUtils::PrintStatus(false, PQerrorMessage(conn));
    if (PQisBusy(conn) == 0) {
      in_loop = true;
      PGresult *result = PQgetResult(conn);
      while (result != NULL) {
        DBPGResult dbresult(result);
        if (dbresult.hasError()) {
          if (job->callback != NULL)
            job->callback->QueryError(dbresult.getError(), job->query);
        } else {
          if (job->callback != NULL)
            job->callback->QueryResult(&dbresult, job->query);
        };
        result = PQgetResult(conn);
      };
      if (job->callback)
        job->callback->QueryDone();
      jobs.pop_front();
      delete job;
      in_loop = false;
      Loop();
    };
  } else if (jobs.empty() == false) {
    conn = PQconnectdb(connstring.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
      TermUtils::PrintStatus(false, PQerrorMessage(conn));
      PQfinish(conn);
      conn = NULL;
    } else {
      PQsetnonblocking(conn, 1);
      if (autocommit) {
        PQJob* autocommit_job = new PQJob("SET AUTOCOMMIT = ON");
        jobs.push_front(autocommit_job);
      };
      if (event == NULL) {
        event = event_new((module) ? module->GetEventBase() : Global::Get()->GetEventBase(), PQsocket(conn), EV_READ|EV_PERSIST, NonBlockingPostGres::EventCallback, this);
        event_add(event, NULL);
      };
      Loop();
    };
  };
};

};
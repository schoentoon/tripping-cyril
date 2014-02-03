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

#ifndef _DATABASE_H
#define _DATABASE_H

#include "String.h"
#include "Module.h"
#include "ShouldDelete.h"

namespace trippingcyril {

class DBResult {
public:
  virtual ~DBResult() {};
  virtual int columns() = 0;
  virtual int rows() = 0;
  virtual String getValue(int row, int column) = 0;
  virtual bool hasError() = 0;
  virtual String getError() = 0;
  virtual bool isNull(int row, int column) = 0;
};

class NonBlockingDBResult : public DBResult {
public:
  NonBlockingDBResult() {};
  virtual int columns() { return 0; };
  virtual int rows() { return 0; };
  virtual String getValue(int row, int column) { return ""; };
  virtual bool hasError() { return false; };
  virtual String getError() { return ""; };
  virtual bool isNull(int row, int column) { return true; };
};

class DBCallback : public ShouldDelete {
public:
  virtual ~DBCallback() {};
  virtual void QueryResult(DBResult* result, const String& query) = 0;
  virtual void QueryError(const String& error, const String& query) = 0;
  virtual void QueryDone() {};
};

class Database {
public:
  Database(const Module *pModule = NULL)
  : module(pModule) {
    autocommit = false;
  };
  virtual ~Database() {};
  virtual DBResult Select(const String& query, DBCallback *callback = NULL) = 0;
  virtual DBResult Insert(const String& query, DBCallback *callback = NULL) = 0;
  virtual bool isIdle() const = 0;
  void enableAutoCommit() { autocommit = true; };
  const Module* GetModule() const { return module; };
protected:
  const Module *module;
  bool autocommit : 1;
};

};

#endif //_DATABASE_H
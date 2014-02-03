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
  virtual const int columns() const = 0;
  virtual const int rows() const = 0;
  virtual const String getValue(int row, int column) const = 0;
  virtual const bool hasError() const = 0;
  virtual const String getError() const = 0;
  virtual const bool isNull(int row, int column) const = 0;
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
  virtual const DBResult* Select(const String& query, DBCallback *callback = NULL) = 0;
  virtual const DBResult* Insert(const String& query, DBCallback *callback = NULL) = 0;
  virtual bool isIdle() const = 0;
  void enableAutoCommit() { autocommit = true; };
  const Module* GetModule() const { return module; };
protected:
  const Module *module;
  bool autocommit : 1;
};

};

#endif //_DATABASE_H
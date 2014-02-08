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

/**
 * @brief Abstraction for database results
 */
class DBResult {
public:
  virtual ~DBResult() {};
  /** @return The amount of columns in the result */
  virtual const int columns() const = 0;
  /** @return The amount of rows in the result */
  virtual const int rows() const = 0;
  /** @return The value present at row, column */
  virtual const String getValue(int row, int column) const = 0;
  /** @return True if there was an error */
  virtual const bool hasError() const = 0;
  /** @return The error if one occured */
  virtual const String getError() const = 0;
  /** @return True if the field at row, column is null */
  virtual const bool isNull(int row, int column) const = 0;
};

/**
 * @brief Database callback class
 */
class DBCallback : public ShouldDelete {
public:
  virtual ~DBCallback() {};
  /**
   * This method will get called with the results
   * @param result The result of the query
   * @param query The query
   */
  virtual void QueryResult(const DBResult* result, const String& query) = 0;
  /**
   * This method will get called upon errors
   * @param error The error message
   * @param query The query
   */
  virtual void QueryError(const String& error, const String& query) = 0;
  /**
   * This will get called after either QueryResult or QueryError, you could use
   * this as a cleanup method.
   */
  virtual void QueryDone() {};
};

/**
 * @brief Database interface
 */
class Database {
public:
  /**
   * General constructor
   * @param pModule The module to register this database on
   */
  Database(const Module *pModule = NULL)
  : module(pModule) {
    autocommit = false;
    stay_connected = false;
  };
  virtual ~Database() {};
  /**
   * @brief Do or queue (depends on implementation) a select operation
   * @param query The query to execute
   * @param callback The callback you would like bundled with this query
   * @return Either the result of a query or just NULL
   */
  virtual const DBResult* Select(const String& query, DBCallback *callback = NULL) = 0;
  /**
   * @brief Do or queue (depends on implementation) an insert operation
   * @param query The query to execute
   * @param callback The callback you would like bundled with this query
   * @return Either the result of a query or just NULL
   */
  virtual const DBResult* Insert(const String& query, DBCallback *callback = NULL) = 0;
  /** @return True if the database is idle */
  virtual bool isIdle() const = 0;
  /** @brief Enables auto commiting */
  void enableAutoCommit() { autocommit = true; };
  /** @brief Enables auto commiting */
  void disableAutoCommit() { autocommit = false; };
  /** @return The module we registered on */
  const Module* GetModule() const { return module; };
  /** Set the stay connected mode on or off */
  void SetStayConnected(bool b) { stay_connected = b; };
protected:
  const Module *module;
  /**
   * Marks if we should use autocommit or not
   */
  bool autocommit : 1;
  /**
   * Marks if we should stay connected or not
   */
  bool stay_connected : 1;
};

};

#endif //_DATABASE_H
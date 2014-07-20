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

#include <syslog.h>

#include "Files.h"
#include "String.h"

namespace trippingcyril {
  namespace log {

/**
 * @brief Base class to provide logging
 */
class Logger {
public:
  // @cond
  virtual ~Logger() {
  };
  // @endcond
  /**
   * Override this method with your actual logging method
   * @param msg The message to log
   */
  virtual void Log(const String &msg) = 0;
};

/**
 * @brief A simple logger implementation which logs into a file
 */
class FileLogger : public Logger {
public:
  /**
   *  Constructor
   *  @param filename The name of the file to log into
   */
  FileLogger(const String& filename);
  virtual ~FileLogger() {
  };
  virtual void Log(const String &msg) OVERRIDE;
private:
  const String filename;
};

/**
 * @brief A simple logger implementation which logs using the <a href="http://linux.die.net/man/3/syslog">syslog(3)</a> method
 */
class SysLogger : public Logger {
public:
  /**
   * Constructor
   * @param priority The priority to use with the log messages
   */
  SysLogger(int priority);
  virtual ~SysLogger() {
  };
  virtual void Log(const String &msg) OVERRIDE;
private:
  const int priority;
};

  };
};
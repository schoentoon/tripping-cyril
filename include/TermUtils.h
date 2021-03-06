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

#include "String.h"

namespace trippingcyril {

/**
 * @brief Helper class to pretty print stuff to the terminal in a fairly standard
 * way.
 */
class TermUtils {
public:
  /**
   * Pretty prints the status of a boolean and a message
   */
  static void PrintStatus(bool status, const String& msg);
  /**
   * Pretty print an error message
   */
  static void PrintError(const String& msg);
  /**
   * Write the pid to path
   */
  static bool WritePidFile(const String& path);
  /**
   * Print a fancy Tripping Cyril message of the day
   */
  static void PrintMotd();
  /**
   * @return The amount of online processors the system has, useful to determine your amount of threads
   */
  static int GetCPUCount();
  /**
   * @brief Pretty print loading statuses
   */
  class StatusPrinter {
  public:
    /**
     * Will show a [ .. ] message indicator until PrintStatus is called
     */
    StatusPrinter(const String& start_message);
    virtual ~StatusPrinter();
    /**
     * Replace the previously printed [ .. ] message with the regular TermUtils::PrintStatus output
     */
    void PrintStatus(bool status, const String& msg);
  private:
    bool called;
  };
private:
  // @cond
  TermUtils() {};
  // @endcond
};

};
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

#ifndef _STACKTRACE_H
#define _STACKTRACE_H

namespace trippingcyril {

/**
 * @brief Crash related functions
 */
namespace crash {

/**
 * Tell our crash handler to go into a gdb shell on crash, useful for debugging
 */
void SetInteractive(bool b);

/**
 * Tell our crash handler to do a coredump as well
 */
void SetCoreDump(bool b);

/**
 * Actually initialize our crash handler
 * @return true if successfully initialized
 */
bool InitCrashHandler();

};
};

#endif //_STACKTRACE_H
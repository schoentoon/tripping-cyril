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

#ifndef _WRITE_INTERFACE_H
#define _WRITE_INTERFACE_H

#include "String.h"
#include "ShouldDelete.h"

namespace trippingcyril {

/**
 * @brief Writer class to provide a general writing operation so it's easier to
 * write wrapper classes for write operations.
 */
class Writer : public ShouldDelete {
public:
  // @cond
  virtual ~Writer() {
  };
  // @endcond
  /**
   * Override this method with your actual write method
   * @param data The actual data
   * @param len The length of the data
   * @return You should return the amount of bytes written, should be len or < 0 on error
   */
  virtual int Write(const char* data, size_t len) = 0;
  /**
   * Lazy String writing
   */
  inline int WriteString(const String& data) { // Why can't I name this Write() :(
    return Write(data.data(), data.size());
  };
  virtual bool shouldDelete() const { return false; };
};

/**
 * @brief Simple in memory writer that just writes everything to a string
 * @note This will clean itself up if the other end respects ShouldDelete
 */
class StringWriter : public Writer {
public:
  virtual ~StringWriter() {
  };
  virtual int Write(const char* data, size_t len) {
    buffer.append(data, len);
    return len;
  };
  virtual bool shouldDelete() const { return true; };
  String& GetBuffer() { return buffer; };
private:
  String buffer;
};

};

#endif //_WRITE_INTERFACE_H
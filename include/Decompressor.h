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

#ifndef _DECOMPRESSOR_H
#define _DECOMPRESSOR_H

#include "defines.h"

#include <stdint.h>

#include "String.h"
#include "Writer.h"

#ifndef _NO_GZIP
#  include <zlib.h>
#endif //_NO_GZIP

namespace trippingcyril {

/**
 * @brief Abstract decompressor
 */
class Decompressor : public Writer {
public:
  /** General constructor */
  Decompressor();
  /** General deconstructor */
  virtual ~Decompressor();
  /**
   * Feed data into the decompressor to decompress
   * @param data The actual data to decompress
   * @param len The length of the data to decompress
   * @return Amount of uncompressed bytes
   */
  virtual int Write(const char* data, size_t len) = 0;
  /** @return The decompressed data */
  const char* data() const { return buffer.data(); };
  /** @return The length of the decompressed data */
  const size_t size() const { return buffer.size(); };
  /** @return The decompressed data wrapped into a String, probably not printable! */
  const String asString() const { return String(data(), size()); };
  /** @return The amount of bytes you fed into the decompressor */
  const uint64_t totalBytesIn() const { return total_in; };
protected:
  /** The output buffer, when implementing this decompressor put your output data
   * in here
   */
  String buffer;
  /** The amount of bytes that we fed into you, you'll have to increase this yourself
   */
  uint64_t total_in;

  /** Just a hint for the buffer size in case you'er implementing this decompressor */
  static int BUFFER_SIZE;
};

#ifndef _NO_GZIP

/**
 * @brief A gzip implementation of the Decompressor interface
 */
class GZipDecompressor : public Decompressor {
public:
  GZipDecompressor();
  virtual ~GZipDecompressor();
  virtual int Write(const char* data, size_t len);
private:
  // @cond
  z_stream zlib_stream;
  // @endcond
};

#endif //_NO_GZIP

};

#endif // _DECOMPRESSOR_H
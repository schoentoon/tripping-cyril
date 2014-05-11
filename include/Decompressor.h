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

#include <stdint.h>

#include "defines.h"
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
  /** Constructor
   * @param pWriter The underlying writer to write the output to
   */
  Decompressor(Writer* pWriter);
  /** General deconstructor
   * @note ShouldDelete on writer is respected
   */
  virtual ~Decompressor();
  /**
   * Feed data into the decompressor to decompress
   * @param data The actual data to decompress
   * @param len The length of the data to decompress
   * @return Amount of uncompressed bytes
   */
  virtual int Write(const char* data, size_t len) = 0;
  /** @return The amount of bytes you fed into the decompressor */
  const uint64_t totalBytesIn() const { return total_in; };
protected:
  /** The amount of bytes that we fed into you, you'll have to increase this yourself
   */
  uint64_t total_in;
  /** Write your compressed data into this writer */
  Writer* writer;
};

#ifndef _NO_GZIP

/**
 * @brief A gzip implementation of the Decompressor interface
 */
class GZipDecompressor : public Decompressor {
public:
  /** Constructor
   * @param pWriter The underlying writer to write the output to
   * @throws std::runtime_error In case initializing zlib goes wrong
   */
  GZipDecompressor(Writer* pWriter);
  /** General deconstructor
   * @note ShouldDelete on writer is respected
   */
  virtual ~GZipDecompressor();
  /**
   * Feed data into the decompressor to decompress
   * @param data The actual data to decompress
   * @param len The length of the data to decompress
   * @return Amount of uncompressed bytes
   */
  virtual int Write(const char* data, size_t len) OVERRIDE;
private:
  // @cond
  z_stream zlib_stream;
  // @endcond
};

#endif //_NO_GZIP

};

#endif // _DECOMPRESSOR_H
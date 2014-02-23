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

#ifndef _COMPRESSOR_H
#define _COMPRESSOR_H

#include "defines.h"

#include <stdint.h>

#include "String.h"

#include <zlib.h>
#include <lzma.h>

namespace trippingcyril {

/**
 * @brief Abstract compressor
 */
class Compressor {
public:
  /** General constructor */
  Compressor();
  /** General deconstructor */
  virtual ~Compressor();
  /**
   * Feed data into the compressor to compress
   * @param data The actual data to compress
   * @param len The length of the data to compress
   * @return True if succesfully compressed
   */
  virtual bool append(const char* data, size_t len) = 0;
  /** @return The compressed data */
  const char* data() const { return buffer.data(); };
  /** @return The length of the compressed data */
  const size_t size() const { return buffer.size(); };
  /** @return The compressed data wrapped into a String, probably not printable! */
  const String asString() const { return String(data(), size()); };
  /** @return The amount of bytes you fed into the compressor */
  const uint64_t totalBytesIn() const { return total_in; };
protected:
  /** The output buffer, when implementing this compressor put your output data
   * in here
   */
  String buffer;
  /** The amount of bytes that we fed into you, you'll have to increase this yourself
   */
  uint64_t total_in;

  /** Just a hint for the buffer size in case you'er implementing this compressor */
  static int BUFFER_SIZE;
};

#ifndef _NO_GZIP

/**
 * @brief A gzip implementation of the Compressor interface
 */
class GZipCompressor : public Compressor {
public:
  /**
   * General constructor
   * @param level The compression level
   * @param memory_level The internal gzip memory level
   */
  GZipCompressor(int level = 6, int memory_level = 8);
  virtual ~GZipCompressor();
  virtual bool append(const char* data, size_t len);
private:
  // @cond
  z_stream zlib_stream;
  // @endcond
};

#endif //_NO_GZIP

#ifndef _NO_LZMA

/**
 * @brief A lzma implementation of the Compressor interface
 */
class LZMACompressor : public Compressor {
public:
  /**
   * General constructor
   * @param preset The level of compression, should be between 1 and 9 or LZMA_PRESET_EXTREME
   * @param check_type The type of checks the algorithm should apply, see the lzma docs
   */
  LZMACompressor(uint32_t preset = 6, lzma_check check_type = LZMA_CHECK_CRC64);
  virtual ~LZMACompressor();
  virtual bool append(const char* data, size_t len);
private:
  lzma_stream stream;
};

#endif //_NO_LZMA

};

#endif // _COMPRESSOR_H
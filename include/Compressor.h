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

#include <stdint.h>

#include "defines.h"
#include "String.h"
#include "Writer.h"

#ifndef _NO_GZIP
#  include <zlib.h>
#endif //_NO_GZIP

#ifndef _NO_LZMA
#  include <lzma.h>
#endif //_NO_LZMA

namespace trippingcyril {

/**
 * @brief Abstract compressor
 */
class Compressor : public Writer {
public:
  /** Constructor
   * @param pWriter The underlying writer to write the output to
   */
  Compressor(Writer* pWriter);
  /** General deconstructor
   * @note ShouldDelete on writer is respected
   */
  virtual ~Compressor();
  /**
   * Feed data into the compressor to compress
   * @param data The actual data to compress
   * @param len The length of the data to compress
   * @return Amount of output bytes
   */
  virtual int Write(const char* data, size_t len) = 0;
  /** @return The amount of bytes you fed into the compressor */
  const uint64_t totalBytesIn() const { return _total_in; };
  /** @return The underlying writer */
  Writer* getWriter() const { return _writer; };
protected:
  /** The amount of bytes that we fed into you, you'll have to increase this yourself
   */
  uint64_t _total_in;
  /** Write your compressed data into this writer */
  Writer* _writer;
};

#ifndef _NO_GZIP

/**
 * @brief A gzip implementation of the Compressor interface
 */
class GZipCompressor : public Compressor {
public:
  /**
   * General constructor
   * @param pWriter The underlying writer to write the output to
   * @param level The compression level
   * @param memory_level The internal gzip memory level
   * @throws std::runtime_error In case initializing zlib goes wrong
   */
  GZipCompressor(Writer* pWriter, int level = 6, int memory_level = 8);
  virtual ~GZipCompressor();
  int Write(const char* data, size_t len) OVERRIDE;
private:
  // @cond
  z_stream _zlib_stream;
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
   * @param pWriter The underlying writer to write the output to
   * @param preset The level of compression, should be between 1 and 9 or LZMA_PRESET_EXTREME
   * @param check_type The type of checks the algorithm should apply, see the lzma docs
   * @throws std::runtime_error In case initializing liblzma goes wrong
   */
  LZMACompressor(Writer* pWriter, uint32_t preset = 6, lzma_check check_type = LZMA_CHECK_CRC64);
  virtual ~LZMACompressor();
  int Write(const char* data, size_t len) OVERRIDE;
private:
  lzma_stream _stream;
};

#endif //_NO_LZMA

};
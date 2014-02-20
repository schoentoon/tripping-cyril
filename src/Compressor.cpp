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

#include "Compressor.h"

#include <string.h>

namespace trippingcyril {

Compressor::Compressor() {
  total_in = 0;
};

Compressor::~Compressor() {
};

int Compressor::BUFFER_SIZE = 4096;

#define windowBits 15
#define GZIP_ENCODING 16

GZipCompressor::GZipCompressor(int level, int memory_level) {
  bzero(&zlib_stream, sizeof(zlib_stream));
  if (deflateInit2(&zlib_stream,
                   (level > 0 && level < 10) ? level : 6,
                   Z_DEFLATED,
                   windowBits | GZIP_ENCODING,
                   (level > 0 && level < 10) ? level : 8,
                   Z_DEFAULT_STRATEGY) != Z_OK)
    throw "deflateInit2 fucked up :(";
};

GZipCompressor::~GZipCompressor() {
  deflateEnd(&zlib_stream);
};

bool GZipCompressor::append(const char* data, size_t len) {
  zlib_stream.next_in = (Bytef*) data;
  zlib_stream.avail_in = len;
  total_in += len;
  int zlib_ret;
  char buf[BUFFER_SIZE];
  do {
    zlib_stream.next_out = (Bytef*) buf;
    zlib_stream.avail_out = sizeof(buf);
    zlib_stream.total_out = 0;
    switch ((zlib_ret = deflate(&zlib_stream, Z_SYNC_FLUSH))) {
    case Z_OK:
    case Z_STREAM_END:
      buffer.append(buf, zlib_stream.total_out);
      break;
    case Z_BUF_ERROR:
      break;
    case Z_MEM_ERROR:
    case Z_DATA_ERROR:
    case Z_NEED_DICT:
    default:
      return false;
    };
  } while (zlib_stream.avail_out == 0);
  return true;
};

};
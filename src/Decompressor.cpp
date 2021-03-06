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

#include "Decompressor.h"

#include <cstring>
#include <stdexcept>

namespace trippingcyril {

Decompressor::Decompressor(Writer* pWriter)
: _total_in(0)
, _writer(pWriter) {
};

Decompressor::~Decompressor() {
  if (_writer->shouldDelete())
    delete _writer;
};

#ifndef _NO_GZIP

#define windowBits 15
#define GZIP_ENCODING 16

GZipDecompressor::GZipDecompressor(Writer* pWriter)
: Decompressor(pWriter) {
  bzero(&_zlib_stream, sizeof(_zlib_stream));
  if (inflateInit2(&_zlib_stream,
                   windowBits | GZIP_ENCODING) != Z_OK)
    throw std::runtime_error("inflateInit2 returned non-Z_OK");
};

GZipDecompressor::~GZipDecompressor() {
  inflateEnd(&_zlib_stream);
};

int GZipDecompressor::Write(const char* data, size_t len) {
  _zlib_stream.next_in = (Bytef*) data;
  _zlib_stream.avail_in = len;
  _total_in += len;
  int zlib_ret;
  int out_len = 0;
  char buf[BUFFER_SIZE];
  do {
    _zlib_stream.next_out = (Bytef*) buf;
    _zlib_stream.avail_out = sizeof(buf);
    _zlib_stream.total_out = 0;
    switch ((zlib_ret = inflate(&_zlib_stream, Z_SYNC_FLUSH))) {
    case Z_OK:
    case Z_STREAM_END:
      _writer->Write(buf, _zlib_stream.total_out);
      out_len += _zlib_stream.total_out;
      break;
    case Z_BUF_ERROR:
      break;
    case Z_MEM_ERROR:
    case Z_DATA_ERROR:
    case Z_NEED_DICT:
    default:
      return -1;
    };
  } while (_zlib_stream.avail_out == 0);
  return out_len;
};

#endif //_NO_GZIP

};
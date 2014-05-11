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

#include <stdexcept>
#include <cstring>

namespace trippingcyril {

Compressor::Compressor(Writer* pWriter) {
  total_in = 0;
  writer = pWriter;
};

Compressor::~Compressor() {
  if (writer->shouldDelete())
    delete writer;
};

#ifndef _NO_GZIP

#define windowBits 15
#define GZIP_ENCODING 16

GZipCompressor::GZipCompressor(Writer* pWriter, int level, int memory_level)
: Compressor(pWriter) {
  bzero(&zlib_stream, sizeof(zlib_stream));
  if (deflateInit2(&zlib_stream,
                   (level > 0 && level < 10) ? level : 6,
                   Z_DEFLATED,
                   windowBits | GZIP_ENCODING,
                   (level > 0 && level < 10) ? level : 8,
                   Z_DEFAULT_STRATEGY) != Z_OK)
    throw std::runtime_error("deflateInit2 returned non-Z_OK");
};

GZipCompressor::~GZipCompressor() {
  deflateEnd(&zlib_stream);
};

int GZipCompressor::Write(const char* data, size_t len) {
  zlib_stream.next_in = (Bytef*) data;
  zlib_stream.avail_in = len;
  total_in += len;
  int zlib_ret;
  char buf[BUFFER_SIZE];
  int out_len = 0;
  do {
    zlib_stream.next_out = (Bytef*) buf;
    zlib_stream.avail_out = sizeof(buf);
    zlib_stream.total_out = 0;
    switch ((zlib_ret = deflate(&zlib_stream, Z_SYNC_FLUSH))) {
    case Z_OK:
    case Z_STREAM_END:
      writer->Write(buf, zlib_stream.total_out);
      out_len += zlib_stream.total_out;
      break;
    case Z_BUF_ERROR:
      break;
    case Z_MEM_ERROR:
    case Z_DATA_ERROR:
    case Z_NEED_DICT:
    default:
      return -1;
    };
  } while (zlib_stream.avail_out == 0);
  return out_len;
};

#endif //_NO_GZIP

#ifndef _NO_LZMA

LZMACompressor::LZMACompressor(Writer* pWriter, uint32_t preset, lzma_check check_type)
: Compressor(pWriter) {
  bzero(&stream, sizeof(stream));
  if (lzma_easy_encoder(&stream, preset, check_type) != LZMA_OK)
    throw std::runtime_error("lzma_easy_encoder returned non-LZMA_OK");
};

LZMACompressor::~LZMACompressor() {
  lzma_end(&stream);
};

int LZMACompressor::Write(const char* data, size_t len) {
  stream.next_in = (uint8_t*) data;
  stream.avail_in = len;
  total_in += len;
  int lzma_ret;
  int out_len = 0;
  char buf[BUFFER_SIZE];
  do {
    stream.next_out = (uint8_t*) buf;
    stream.avail_out = sizeof(buf);
    stream.total_out = 0;
    switch ((lzma_ret = lzma_code(&stream, LZMA_RUN))) {
    case LZMA_OK:
    case LZMA_STREAM_END:
      writer->Write(buf, stream.total_out);
      out_len += stream.total_out;
      break;
    case LZMA_MEM_ERROR:
    case LZMA_DATA_ERROR:
    default:
      return -1;
    };
  } while (stream.avail_out == 0);
  return out_len;
};

#endif //_NO_LZMA

};
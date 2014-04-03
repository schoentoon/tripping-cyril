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

#include "SimpleHTTPSocket.h"

#include <iostream>
#include <stdlib.h>
#include <strings.h>

namespace trippingcyril {

SimpleHTTPSocket::SimpleHTTPSocket(const Module* module, HTTPCallback* callback)
: Socket(module), parser(this) {
  this->callback = callback;
  SetTCPNoDelay(true);
  SetTimeout(60);
};

#if __cplusplus >= 201103

class HttpLamdbaCallback : public HTTPCallback {
public:
  HttpLamdbaCallback(const HTTPLamdbaCallback &_callback, const HTTPLamdbaErrorCallback &_errorcallback)
  : callback(_callback)
  , errorcallback(_errorcallback) {
  };
  virtual ~HttpLamdbaCallback() {};
  virtual void OnRequestDone(unsigned short responseCode, const map<String, String>& headers, const String& response, const String& url) {
    callback(responseCode, headers, response, url);
  };
  virtual void OnRequestError(int errorCode, const String& url) {
    errorcallback(errorCode, url);
  };
private:
  const HTTPLamdbaCallback callback;
  const HTTPLamdbaErrorCallback errorcallback;
};

SimpleHTTPSocket::SimpleHTTPSocket(const Module* module, const HTTPLamdbaCallback &callback, const HTTPLamdbaErrorCallback &errorcallback)
: SimpleHTTPSocket(module, new HttpLamdbaCallback(callback, errorcallback))
{
};

#endif

SimpleHTTPSocket::~SimpleHTTPSocket() {
  if (callback != NULL && callback->shouldDelete())
    delete callback;
};

bool SimpleHTTPSocket::CrackURL(const String& url, String& host, String& path, unsigned short& port, bool& ssl) {
  String work(url);
  if (work.substr(0, 7) == "http://") {
    ssl = false;
    port = 80;
    work.erase(0, 7);
  } else if (work.substr(0, 8) == "https://") {
    ssl = true;
    port = 443;
    work.erase(0, 8);
  } else
    return false;
  String::size_type uPos = work.find('/');
  if (uPos == String::npos) {
    host = work;
    path = "/";
  } else {
    host = work.substr(0, uPos);
    path = work.substr(uPos + 1);
  };
  uPos = work.find(':');
  if (uPos != String::npos) {
    port = String(host.substr(uPos + 1)).ToUShort();
    host.erase(uPos);
  };
  if (host.empty())
    return false;
  for (String::size_type c = 0; c < host.size(); c++) {
    if (!isalnum(host[c]) && host[c] != '.' && host[c] != '-')
      return false;
  };
  for (String::size_type c = 0; c < path.size(); c++) {
    if (!path[c] || path[c] == '\n' || path[c] == '\r')
      return false;
  };
  return true;
};

void SimpleHTTPSocket::MakeRequestHeaders(const String& method, const String& host, const String& path, unsigned short port, bool ssl) {
  buffer = method + " ";
  if (path.empty())
    buffer += "/";
  else if (path.substr(0, 1) == "/")
    buffer += path;
  else
    buffer += "/" + path;
  buffer += " HTTP/1.1\r\n";
  buffer += "Host: " + host + ((port == 80 && ssl == false) || (port == 443 || ssl == true) ? "" : ":" + String(port)) + "\r\n";
  buffer += "User-Agent: Tripping Cyril\r\n";
#ifndef _NO_GZIP
  buffer += "Accept-Encoding: gzip\r\n";
#else
  buffer += "Accept-Encoding: identity\r\n";
#endif //_NO_GZIP
  buffer += "Connection: Close\r\n";
  for (map<String, String>::iterator iter = extraHeaders.begin(); iter != extraHeaders.end(); ++iter)
    buffer += iter->first + ": " + iter->second + "\r\n";
};

bool SimpleHTTPSocket::Get(const String& url) {
  if (this->url.empty() == false)
    return false;
  String path, host;
  unsigned short port;
  bool ssl;
  if (CrackURL(url, host, path, port, ssl) == false)
    return false;
  this->url = url;
  MakeRequestHeaders("GET", host, path, port, ssl);
  buffer += "\r\n";
  Connect(host, port, ssl);
  return true;
};

bool SimpleHTTPSocket::Post(const String& url, const String& postData, const String& type) {
  if (this->url.empty() == false)
    return false;
  String path, host;
  unsigned short port;
  bool ssl;
  if (CrackURL(url, host, path, port, ssl) == false)
    return false;
  this->url = url;
  extraHeaders["Content-Type"] = type;
  extraHeaders["Content-Length"] = String(postData.size());
  MakeRequestHeaders("POST", host, path, port, ssl);
  buffer += "\r\n";
  buffer += postData;
  buffer += "\r\n";
  Connect(host, port, ssl);
  return true;
};

void SimpleHTTPSocket::Connected() {
  WriteString(buffer);
  buffer.clear();
  SetReadLine(true);
};

void SimpleHTTPSocket::Disconnected() {
  if (buffer.empty() == false && parser.responseCode != 0)
    OnRequestDone(parser.responseCode, parser.headers, buffer);
};

void SimpleHTTPSocket::Timeout() {
  OnRequestError(TIMEOUT);
};

void SimpleHTTPSocket::OnRequestDone(unsigned short responseCode, map<String, String>& headers, const String& response) {
  if (callback != NULL) {
    callback->OnRequestDone(responseCode, headers, response, url);
    if (callback->shouldDelete())
      delete callback;
    callback = NULL;
  };
};

void SimpleHTTPSocket::OnRequestError(int errorCode) {
  if (callback != NULL) {
    callback->OnRequestError(errorCode, url);
    if (callback->shouldDelete())
      delete callback;
    callback = NULL;
  };
};

void SimpleHTTPSocket::ReadLine(const String& data) {
  if (parser.headersDone == false)
    parser.ParseLine(data);
  else if (parser.chunked == true && parser.current_chunk <= 0 && data.empty() == false) {
    int chunk = data.ToInt(16);
    if (chunk > 0) {
      parser.current_chunk = chunk;
      SetReadLine(false);
    } else { // Chunk length 0 indicates that it is done.
      OnRequestDone(parser.responseCode, parser.headers, buffer);
      Close();
    };
  };
};

size_t SimpleHTTPSocket::ReadData(const char* data, size_t len) {
  if (parser.chunked == true && parser.current_chunk > 0) {
    len = std::min(len, (size_t) parser.current_chunk);
    if (parser.IsCompressed() == true) {
#ifndef _NO_GZIP
      len = Decompress(data, len);
#else
      perror("You compiled without gzip support but this is a compressed stream anyway?");
#endif //_NO_GZIP
    } else
      buffer.append(data, len);
    parser.current_chunk -= len;
    if (parser.current_chunk <= 0)
      SetReadLine(true);
  } else if (parser.chunked == false) {
    if (parser.IsCompressed() == true) {
#ifndef _NO_GZIP
      len = Decompress(data, len);
#else
      perror("You compiled without gzip support but this is a compressed stream anyway?");
#endif //_NO_GZIP
    } else
      buffer.append(data, len);
  };
  if (parser.contentLength > 0) {
    if ((parser.IsCompressed() == false && buffer.size() >= (size_t) parser.contentLength)
#ifndef _NO_GZIP
      || (parser.IsCompressed() == true && parser.zlib_stream->total_in >= (size_t) parser.contentLength)
#endif //_NO_GZIP
    ) {
      OnRequestDone(parser.responseCode, parser.headers, buffer);
      Close();
    };
  };
  return len;
};

#ifndef _NO_GZIP

#define CHUNK_SIZE 16384

size_t SimpleHTTPSocket::Decompress(const char* data, size_t len) {
  if (parser.IsCompressed() == false)
    return 0;
  parser.zlib_stream->next_in = (Bytef*) data;
  parser.zlib_stream->avail_in = len;
  int zlib_ret;
  char buf[CHUNK_SIZE];
  do {
    parser.zlib_stream->next_out = (unsigned char*) buf;
    parser.zlib_stream->avail_out = sizeof(buf);
    parser.zlib_stream->total_out = 0;
    switch ((zlib_ret = inflate(parser.zlib_stream, Z_NO_FLUSH))) {
    case Z_OK:
    case Z_STREAM_END:
      buffer.append(buf, parser.zlib_stream->total_out);
      break;
    case Z_BUF_ERROR: // Non fatal error
      break;
    case Z_MEM_ERROR:
    case Z_DATA_ERROR:
    case Z_NEED_DICT:
    default:
      OnRequestError(DECOMPESSION_ERROR);
      Close();
      return 0;
    };
  } while (parser.zlib_stream->avail_out == 0);
  return len - parser.zlib_stream->avail_in;
};

#endif //_NO_GZIP

SimpleHTTPSocket::HTTPParser::HTTPParser(SimpleHTTPSocket* socket) {
  this->socket = socket;
  responseCode = 0;
  contentLength = -1;
  chunked = false;
  current_chunk = -1;
  headersDone = false;
#ifndef _NO_GZIP
  zlib_stream = NULL;
#endif //_NO_GZIP
};

SimpleHTTPSocket::HTTPParser::~HTTPParser() {
#ifndef _NO_GZIP
  if (zlib_stream != NULL) {
    inflateEnd(zlib_stream);
    free(zlib_stream);
  };
#endif //_NO_GZIP
};

bool SimpleHTTPSocket::HTTPParser::ParseLine(const String& line) {
  if (responseCode == 0) {
    if (line.size() > 8) {
      String responseCodeStr(line.substr(8));
      responseCode = responseCodeStr.ToUShort();
      return (responseCode >= 100 && responseCode < 600);
    };
    return false;
  } else {
    if (line.empty()) {
      headersDone = true;
      if (chunked == false)
        socket->SetReadLine(false);
      return true;
    };
    String::size_type k = line.find(':');
    if (k != String::npos) {
      String key(line.substr(0, k));
      String value(line.substr(k + 1));
      key.Trim();
      value.Trim();
      headers[key] = value;
      key.MakeLower();
      value.MakeLower();
      if (contentLength == -1 && key == "content-length")
        contentLength = value.ToLong();
      else if (chunked == false && key == "transfer-encoding" && value == "chunked")
        chunked = true;
#ifndef _NO_GZIP
      else if (zlib_stream == NULL && key == "content-encoding" && value == "gzip") {
        zlib_stream = (z_stream*) malloc(sizeof(z_stream));
        bzero(zlib_stream, sizeof(z_stream));
        if (inflateInit2(zlib_stream, 15 + 32) != Z_OK) {
          std::cerr << "inflateInit2 went wrong!" << std::endl;
          abort();
        };
      };
#endif //_NO_GZIP
      return true;
    }
  };
  return false;
};

};
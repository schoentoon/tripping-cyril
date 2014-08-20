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

#include "TermUtils.h"

#include <iostream>
#include <stdlib.h>
#include <strings.h>

namespace trippingcyril {
  namespace net {
    namespace http {

SimpleHTTPSocket::SimpleHTTPSocket(const Module* module, HTTPCallback* callback)
: Socket(module)
, _sent_data(false)
, _callback(callback)
, _parser(this) {
  SetTCPNoDelay(true);
  SetTimeout(60);
};

#if __cplusplus >= 201103

class HttpLamdbaCallback : public HTTPCallback {
public:
  HttpLamdbaCallback(const HTTPLamdbaCallback &_callback, const HTTPLamdbaErrorCallback &_errorcallback)
  : _callback(_callback)
  , _errorcallback(_errorcallback) {
  };
  virtual ~HttpLamdbaCallback() {};
  virtual void OnRequestDone(unsigned short responseCode, const map<String, String>& headers, const String& response, const String& url) {
    if (_callback)
      _callback(responseCode, headers, response, url);
  };
  virtual void OnRequestError(int errorCode, const String& url) {
    if (_errorcallback)
      _errorcallback(errorCode, url);
  };
private:
  const HTTPLamdbaCallback _callback;
  const HTTPLamdbaErrorCallback _errorcallback;
};

SimpleHTTPSocket::SimpleHTTPSocket(const Module* module, const HTTPLamdbaCallback &callback, const HTTPLamdbaErrorCallback &errorcallback)
: SimpleHTTPSocket(module, new HttpLamdbaCallback(callback, errorcallback))
{
};

#endif

SimpleHTTPSocket::~SimpleHTTPSocket() {
  if (_callback != NULL && _callback->shouldDelete())
    delete _callback;
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
  _buffer = method + ' ';
  if (path.empty())
    _buffer += '/';
  else if (path.substr(0, 1) == "/")
    _buffer += path;
  else
    _buffer += '/' + path;
  _buffer += " HTTP/1.1\r\n";
  _buffer += "Host: " + host + ((port == 80 && ssl == false) || (port == 443 || ssl == true) ? "" : ":" + String(port)) + "\r\n";
  _buffer += "User-Agent: Tripping Cyril\r\n";
#ifndef _NO_GZIP
  _buffer += "Accept-Encoding: gzip\r\n";
#else
  _buffer += "Accept-Encoding: identity\r\n";
#endif //_NO_GZIP
  _buffer += "Connection: Close\r\n";
  for (map<String, String>::iterator iter = _extraHeaders.begin(); iter != _extraHeaders.end(); ++iter)
    _buffer += iter->first + ": " + iter->second + "\r\n";
};

bool SimpleHTTPSocket::Get(const String& url) {
  if (this->_url.empty() == false)
    return false;
  String path, host;
  unsigned short port;
  bool ssl;
  if (CrackURL(url, host, path, port, ssl) == false)
    return false;
  this->_url = url;
  MakeRequestHeaders("GET", host, path, port, ssl);
  _buffer += "\r\n";
  Connect(host, port, ssl);
  return true;
};

bool SimpleHTTPSocket::Post(const String& url, const String& postData, const String& type) {
  if (this->_url.empty() == false)
    return false;
  String path, host;
  unsigned short port;
  bool ssl;
  if (CrackURL(url, host, path, port, ssl) == false)
    return false;
  this->_url = url;
  if (!type.empty())
    _extraHeaders["Content-Type"] = type;
  _extraHeaders["Content-Length"] = String(postData.size());
  MakeRequestHeaders("POST", host, path, port, ssl);
  _buffer += "\r\n";
  _buffer += postData;
  _buffer += "\r\n";
  Connect(host, port, ssl);
  return true;
};

bool SimpleHTTPSocket::Patch(const String& url, const String& patchData, const String& type) {
  if (this->_url.empty() == false)
    return false;
  String path, host;
  unsigned short port;
  bool ssl;
  if (CrackURL(url, host, path, port, ssl) == false)
    return false;
  this->_url = url;
  if (!type.empty())
    _extraHeaders["Content-Type"] = type;
  _extraHeaders["Content-Length"] = String(patchData.size());
  MakeRequestHeaders("PATCH", host, path, port, ssl);
  _buffer += "\r\n";
  _buffer += patchData;
  _buffer += "\r\n";
  Connect(host, port, ssl);
  return true;
};

bool SimpleHTTPSocket::Put(const String& url, const String& putData, const String& type) {
  if (this->_url.empty() == false)
    return false;
  String path, host;
  unsigned short port;
  bool ssl;
  if (CrackURL(url, host, path, port, ssl) == false)
    return false;
  this->_url = url;
  if (!type.empty())
    _extraHeaders["Content-Type"] = type;
  _extraHeaders["Content-Length"] = String(putData.size());
  MakeRequestHeaders("PUT", host, path, port, ssl);
  _buffer += "\r\n";
  _buffer += putData;
  _buffer += "\r\n";
  Connect(host, port, ssl);
  return true;
};

bool SimpleHTTPSocket::Delete(const String& url, const String& deleteData, const String& type) {
  if (this->_url.empty() == false)
    return false;
  String path, host;
  unsigned short port;
  bool ssl;
  if (CrackURL(url, host, path, port, ssl) == false)
    return false;
  this->_url = url;
  if (!type.empty())
    _extraHeaders["Content-Type"] = type;
  _extraHeaders["Content-Length"] = String(deleteData.size());
  MakeRequestHeaders("DELETE", host, path, port, ssl);
  _buffer += "\r\n";
  _buffer += deleteData;
  _buffer += "\r\n";
  Connect(host, port, ssl);
  return true;
};

void SimpleHTTPSocket::Connected() {
  WriteString(_buffer);
  _buffer.clear();
  SetReadLine(true);
};

void SimpleHTTPSocket::Disconnected() {
  if (_buffer.empty() == false && _parser._responseCode != 0)
    OnRequestDone(_parser._responseCode, _parser._headers, _buffer);
};

void SimpleHTTPSocket::Timeout() {
  OnRequestError(TIMEOUT);
};

void SimpleHTTPSocket::OnRequestDone(unsigned short responseCode, map<String, String>& headers, const String& response) {
  if (_callback != NULL) {
    _callback->OnRequestDone(responseCode, headers, response, _url);
    if (_callback->shouldDelete())
      delete _callback;
    _callback = NULL;
  };
};

void SimpleHTTPSocket::OnRequestError(int errorCode) {
  if (_callback != NULL) {
    _callback->OnRequestError(errorCode, _url);
    if (_callback->shouldDelete())
      delete _callback;
    _callback = NULL;
  };
};

void SimpleHTTPSocket::ReadLine(const String& data) {
  if (_sent_data == false) {
    OnRequestError(GOT_RESPONSE_TOO_EARLY);
    Close();
  } else if (_parser._headersDone == false)
    _parser.ParseLine(data);
  else if (_parser._chunked == true && _parser._current_chunk <= 0 && data.empty() == false) {
    int chunk = data.ToInt(16);
    if (chunk > 0) {
      _parser._current_chunk = chunk;
      SetReadLine(false);
    } else { // Chunk length 0 indicates that it is done.
      OnRequestDone(_parser._responseCode, _parser._headers, _buffer);
      Close();
    };
  };
};

size_t SimpleHTTPSocket::ReadData(const char* data, size_t len) {
  if (_parser._chunked == true && _parser._current_chunk > 0) {
    len = std::min(len, (size_t) _parser._current_chunk);
#ifndef _NO_GZIP
    if (_parser.IsCompressed() == true) {
      len = Decompress(data, len);
    } else
#endif //_NO_GZIP
    {
      _buffer.append(data, len);
    };
    _parser._current_chunk -= len;
    if (_parser._current_chunk <= 0)
      SetReadLine(true);
  } else if (_parser._chunked == false) {
#ifndef _NO_GZIP
    if (_parser.IsCompressed() == true) {
      len = Decompress(data, len);
    } else
#endif //_NO_GZIP
    {
      _buffer.append(data, len);
    };
  };
  if (_parser._contentLength > 0) {
#ifndef _NO_GZIP
    if ((_parser.IsCompressed() == false && _buffer.size() >= (size_t) _parser._contentLength)
      || (_parser.IsCompressed() == true && _parser._zlib_stream->total_in >= (size_t) _parser._contentLength))
#endif //_NO_GZIP
    {
      OnRequestDone(_parser._responseCode, _parser._headers, _buffer);
      Close();
    };
  };
  return len;
};

void SimpleHTTPSocket::OnWrite(size_t bytes_left) {
  if (bytes_left == 0)
    _sent_data = true;
};

#ifndef _NO_GZIP

#define CHUNK_SIZE 16384

size_t SimpleHTTPSocket::Decompress(const char* data, size_t len) {
  if (_parser.IsCompressed() == false)
    return 0;
  _parser._zlib_stream->next_in = (Bytef*) data;
  _parser._zlib_stream->avail_in = len;
  int zlib_ret;
  char buf[CHUNK_SIZE];
  do {
    _parser._zlib_stream->next_out = (unsigned char*) buf;
    _parser._zlib_stream->avail_out = sizeof(buf);
    _parser._zlib_stream->total_out = 0;
    switch ((zlib_ret = inflate(_parser._zlib_stream, Z_NO_FLUSH))) {
    case Z_OK:
    case Z_STREAM_END:
      _buffer.append(buf, _parser._zlib_stream->total_out);
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
  } while (_parser._zlib_stream->avail_out == 0);
  return len - _parser._zlib_stream->avail_in;
};

#endif //_NO_GZIP

SimpleHTTPSocket::HTTPParser::HTTPParser(SimpleHTTPSocket* socket)
: _responseCode(0)
, _contentLength(-1)
, _chunked(false)
, _current_chunk(-1)
, _headersDone(false)
, _socket(socket)
#ifndef _NO_GZIP
, _zlib_stream(NULL)
#endif //_NO_GZIP
{
};

SimpleHTTPSocket::HTTPParser::~HTTPParser() {
#ifndef _NO_GZIP
  if (_zlib_stream != NULL) {
    inflateEnd(_zlib_stream);
    free(_zlib_stream);
  };
#endif //_NO_GZIP
};

bool SimpleHTTPSocket::HTTPParser::ParseLine(const String& line) {
  if (_responseCode == 0) {
    if (line.size() > 8) {
      String responseCodeStr(line.substr(8));
      _responseCode = responseCodeStr.ToUShort();
      return (_responseCode >= 100 && _responseCode < 600);
    };
    return false;
  } else {
    if (line.empty()) {
      _headersDone = true;
      if (_chunked == false)
        _socket->SetReadLine(false);
      return true;
    };
    String::size_type k = line.find(':');
    if (k != String::npos) {
      String key(line.substr(0, k));
      String value(line.substr(k + 1));
      key.Trim();
      value.Trim();
      _headers[key] = value;
      key.MakeLower();
      value.MakeLower();
      if (_contentLength == -1 && key == "content-length")
        _contentLength = value.ToLong();
      else if (_chunked == false && key == "transfer-encoding" && value == "chunked")
        _chunked = true;
#ifndef _NO_GZIP
      else if (_zlib_stream == NULL && key == "content-encoding" && value == "gzip") {
        _zlib_stream = (z_stream*) malloc(sizeof(z_stream));
        bzero(_zlib_stream, sizeof(z_stream));
        if (inflateInit2(_zlib_stream, 15 + 32) != Z_OK) {
          TermUtils::PrintError("inflateInit2 went wrong!");
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
  };
};
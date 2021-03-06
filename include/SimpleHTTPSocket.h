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

#include <map>

#ifndef _NO_GZIP
#  include <zlib.h>
#endif //_NO_GZIP

#if __cplusplus >= 201103
#  include <functional>
#endif

#include "defines.h"
#include "String.h"
#include "Socket.h"
#include "ShouldDelete.h"

using namespace std;

namespace trippingcyril {
  namespace net {
    namespace http {

#if __cplusplus >= 201103
typedef std::function<void(unsigned short responseCode, const map<String, String>& headers, const String& response, const String& url)> HTTPLamdbaCallback;
typedef std::function<void(int errorCode, const String& url)> HTTPLamdbaErrorCallback;
#endif

/**
 * @brief Callback class for our SimpleHTTPSocket
 */
class HTTPCallback : public ShouldDelete {
public:
  virtual ~HTTPCallback() {};
  /**
   * Called if our request finished without any http parsing errors
   * @param responseCode The http response code, currently all response codes will be passed to this
   * @param headers The http response headers
   * @param response The actual http response, in case of binary data depend on .data() and .size()
   * @param url The url this is a response to
   */
  virtual void OnRequestDone(unsigned short responseCode, const map<String, String>& headers, const String& response, const String& url) = 0;
  /**
   * Called in case of any internal errors
   * @param errorCode Our internal error code
   * @param url The url this error was for
   * @see SimpleHTTPSocket::DECOMPESSION_ERROR
   * @see SimpleHTTPSocket::TIMEOUT
   */
  virtual void OnRequestError(int errorCode, const String& url) {};
};

/**
 * @brief A simple http client
 */
class SimpleHTTPSocket : public Socket {
public:
  /**
   * General constructor and the place to register your callback
   */
  SimpleHTTPSocket(const Module* module, HTTPCallback* callback = NULL);
#if __cplusplus >= 201103
  /**
   * General constructor that uses lamdba functions instead of a general callback
   * @note Requires C++11
   */
  SimpleHTTPSocket(const Module* module
                  ,const HTTPLamdbaCallback &callback
                  ,const HTTPLamdbaErrorCallback &errorcallback = nullptr);
#endif
  /** Deconstructor */
  virtual ~SimpleHTTPSocket();
  /**
   * Do a simple HTTP GET request
   * @param url The url to request
   * @return True if url was succesfully parsed and the request is started
   */
  bool Get(const String& url);
  /**
   * Do a simple HTTP POST request
   * @param url The url to request
   * @param postData The raw post data to send
   * @param type The Content-Type header to set
   * @return True if url was succesfully parsed and the request is started
   */
  bool Post(const String& url, const String& postData, const String& type = "");
  /**
   * Do a simple HTTP PATCH request
   * @param url The url to request
   * @param patchData the raw patch data to send
   * @param type The Content-Type header to set
   * @return True if url was succesfully parsed and the request is started
   */
  bool Patch(const String& url, const String& patchData, const String& type = "");
  /**
   * Do a simple HTTP PUT request
   * @param url The url to request
   * @param putData the raw put data to send
   * @param type The Content-Type header to set
   * @return True if url was succesfully parsed and the request is started
   */
  bool Put(const String& url, const String& putData, const String& type = "");
  /**
   * Do a simple HTTP DELETE request
   * @param url The url to request
   * @param deleteData the raw put data to send
   * @param type The Content-Type header to set
   * @return True if url was succesfully parsed and the request is started
   */
  bool Delete(const String& url, const String& deleteData, const String& type = "");
  /**
   * Set any custom headers to send with our request, must be called before Get()
   * or Post()
   * @param key The key for your custom header
   * @param value The value for your custom header
   */
  void setExtraHeader(const String& key, const String& value) {
    _extraHeaders[key] = value;
  };
  /** @return The url requested */
  String getURL() const { return _url; };
  /**
   * Error code in case of any decompression errors. In case you see this error
   * often please report a bug over at https://github.com/schoentoon/tripping-cyril/issues
   * @see HTTPCallback::OnRequestError
   */
  static const int DECOMPESSION_ERROR = -1;
  /**
   * Error code in case of a general socket timeout.
   * @see HTTPCallback::OnRequestError
   * @see Socket::Timeout
   */
  static const int TIMEOUT = -2;
  /**
   * Error code for when we get a response before we sent out our full request,
   * which of course shouldn't be happening!
   */
  static const int GOT_RESPONSE_TOO_EARLY = -3;
protected:
  /**
   * Connected event from the Socket class, make sure to call this implemention
   * in case of overriding this class as it sends of the actual request etc.
   */
  void Connected() OVERRIDE;
  /**
   * Disconnected event from the Socket class, make sure to call this implemention
   * in case of overriding this class as it may call one of the HTTPCallback methods.
   */
  void Disconnected() OVERRIDE;
  /**
   * Timeout event from the Socket class.
   * @see TIMEOUT
   */
  void Timeout() OVERRIDE;
  // @cond
  void ReadLine(const String& data) OVERRIDE;
  size_t ReadData(const char* data, size_t len) OVERRIDE;
  void OnWrite(size_t bytes_left) OVERRIDE;
  virtual void OnRequestDone(unsigned short responseCode, map<String, String>& headers, const String& response);
  virtual void OnRequestError(int errorCode);
private:
  static bool CrackURL(const String& url, String& host, String& path, unsigned short& port, bool& ssl);
  void MakeRequestHeaders(const String& method, const String& host, const String& path, unsigned short port, bool ssl);
#ifndef _NO_GZIP
  size_t Decompress(const char* data, size_t len);
#endif //_NO_GZIP
  map<String, String> _extraHeaders;
  String _url;
  String _buffer;
  bool _sent_data : 1;
  HTTPCallback* _callback;
  class HTTPParser {
  public:
    HTTPParser(SimpleHTTPSocket* socket);
    virtual ~HTTPParser();
    bool ParseLine(const String& line);
#ifndef _NO_GZIP
    bool IsCompressed() const {
      return _zlib_stream != NULL;
    };
#endif //_NO_GZIP
    unsigned short _responseCode;
    long _contentLength;
    bool _chunked;
    int _current_chunk;
    bool _headersDone;
    map<String, String> _headers;
    SimpleHTTPSocket* _socket;
#ifndef _NO_GZIP
    z_stream* _zlib_stream;
#endif //_NO_GZIP
  };
  HTTPParser _parser;
  // @endcond
};

    };
  };
};
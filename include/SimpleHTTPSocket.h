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

#ifndef _HTTP_SOCKET_H
#define _HTTP_SOCKET_H

#include <map>

#include "String.h"
#include "Socket.h"

using namespace std;

namespace trippingcyril {

class HTTPCallback {
public:
  virtual ~HTTPCallback() {};
  virtual void OnRequestDone(unsigned short responseCode, const map<String, String>& headers, const String& response, const String& url) = 0;
  virtual bool shouldDelete() { return true; };
};

class SimpleHTTPSocket : public Socket {
public:
  SimpleHTTPSocket(Module* module, HTTPCallback* callback = NULL);
  virtual ~SimpleHTTPSocket();
  static bool CrackURL(const String& url, String& host, String& path, unsigned short& port, bool& ssl);
  bool Get(const String& url);
  bool Post(const String& url, const String& postData, const String& type);
  void setExtraHeader(const String& key, const String& value) {
    extraHeaders[key] = value;
  };
  String getURL() const { return url; };
protected:
  void Connected();
  void Disconnected();
  void Timeout() {};
  void ReadLine(const String& data);
  size_t ReadData(const char* data, size_t len);
  virtual void OnRequestDone(unsigned short responseCode, map<String, String>& headers, const String& response);
private:
  void MakeRequestHeaders(bool post, const String& host, const String& path, unsigned short port, bool ssl);
  map<String, String> extraHeaders;
  String url;
  String buffer;
  HTTPCallback* callback;
  class HTTPParser {
  public:
    HTTPParser(SimpleHTTPSocket* socket);
    virtual ~HTTPParser();
    bool ParseLine(const String& line);
    unsigned short responseCode;
    long contentLength;
    bool chunked;
    int current_chunk;
    bool headersDone;
    map<String, String> headers;
    SimpleHTTPSocket* socket;
  };
  HTTPParser parser;
};

};

#endif //_HTTP_SOCKET_H
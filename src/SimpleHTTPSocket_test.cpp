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

#include <gtest/gtest.h>

#include <deque>

#include "LibEventHelper.h"

#include "SimpleHTTPSocket.h"
#include "Files.h"

namespace test {

class SimpleHTTPSocket : public LibEventTest {
};

class TestHTTPCallback : public trippingcyril::HTTPCallback {
public:
  TestHTTPCallback() {
    expectedResponseCode = 0;
    expectedResponse = "";
    expectedErrorCode = 0;
    keepAround = false;
    done = NULL;
  };
  void OnRequestDone(unsigned short responseCode, const map<String, String>& headers, const String& response, const String& url) {
    if (expectedResponseCode > 0) {
      EXPECT_EQ(expectedResponseCode, responseCode);
    };
    if (expectedResponse.empty() == false) {
      EXPECT_EQ(expectedResponse, response);
    };
    for (map<String, String>::iterator it = expectedHeaders.begin(); it != expectedHeaders.end(); it++) {
      map<String, String>::const_iterator iter = headers.find(it->first);
      if (iter == headers.end()) {
        ADD_FAILURE() << "Expected " << it->first << " in our response, but we couldn't find it.";
      } else {
        EXPECT_EQ(it->second, iter->second);
      };
    }
    if (done)
      *done = true;
  };
  void OnRequestError(int errorCode, const String& url) {
    if (expectedErrorCode != 0) {
      EXPECT_EQ(expectedErrorCode, errorCode);
    }
    if (done)
      *done = true;
  };
  bool shouldDelete() const { return !keepAround; };
  bool keepAround;
  map<String, String> expectedHeaders;
  unsigned int expectedResponseCode;
  int expectedErrorCode;
  String expectedResponse;
  bool *done;
};

TEST_F(SimpleHTTPSocket, SimpleReply) {
  TestHTTPCallback* callback = new TestHTTPCallback;
  callback->expectedResponseCode = 200;
  callback->expectedHeaders["Access-Control-Allow-Origin"] = "*";
  callback->expectedResponse = "{\"leet\":1337\r\n,\"numbers\": [1,2,3,4]}";
  bool done = false;
  callback->done = &done;
  trippingcyril::SimpleHTTPSocket* socket = new trippingcyril::SimpleHTTPSocket(event_base, callback);
  EXPECT_FALSE(socket->IsConnected());
  EXPECT_TRUE(socket->Get("http://127.0.0.1/test"));
  event_base->Event(socket, BEV_EVENT_CONNECTED);
  EXPECT_TRUE(socket->IsConnected());
  String data = "HTTP/1.0 200 OK\r\n"
  "Content-Type: application/json\r\n"
  "Content-Length: 36\r\n"
  "Access-Control-Allow-Origin: *\r\n"
  "Connection: close\r\n\r\n{\"leet\":1337\r\n"
  ",\"numbers\": [1,2,3,4]}";
  event_base->AddData(data, socket);
  while (done == false)
    event_base->Read(socket);
  event_base->Event(socket, BEV_EVENT_ERROR);
  EXPECT_DEATH(delete socket, "");
  EXPECT_DEATH(delete callback, "");
};

TEST_F(SimpleHTTPSocket, Chunked) {
  TestHTTPCallback* callback = new TestHTTPCallback;
  callback->expectedResponseCode = 200;
  callback->expectedHeaders["Transfer-Encoding"] = "chunked";
  callback->expectedResponse = "This is just a simple test";
  bool done = false;
  callback->done = &done;
  trippingcyril::SimpleHTTPSocket* socket = new trippingcyril::SimpleHTTPSocket(event_base, callback);
  EXPECT_FALSE(socket->IsConnected());
  EXPECT_TRUE(socket->Get("http://127.0.0.1/test_chunked"));
  event_base->Event(socket, BEV_EVENT_CONNECTED);
  EXPECT_TRUE(socket->IsConnected());
  String data = "HTTP/1.0 200 OK\r\n"
  "Transfer-Encoding: chunked\r\n"
  "Connection: close\r\n\r\n"
  "04\r\nThis\r\n03\r\n is\r\n13\r\n just a simple test\r\n0\r\n";
  event_base->AddData(data, socket);
  while (done == false)
    event_base->Read(socket);
  event_base->Event(socket, BEV_EVENT_ERROR);
  EXPECT_DEATH(delete socket, "");
  EXPECT_DEATH(delete callback, "");
};

#ifndef _NO_GZIP

TEST_F(SimpleHTTPSocket, GZip) {
  TestHTTPCallback* callback = new TestHTTPCallback;
  callback->expectedResponseCode = 200;
  callback->expectedResponse = "Lorem ipsum dolor sit amet, consectetur adipisicing "
  "elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim "
  "ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
  "commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse "
  "cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, "
  "sunt in culpa qui officia deserunt mollit anim id est laborum.\n";
  bool done = false;
  callback->done = &done;
  trippingcyril::SimpleHTTPSocket* socket = new trippingcyril::SimpleHTTPSocket(event_base, callback);
  EXPECT_FALSE(socket->IsConnected());
  EXPECT_TRUE(socket->Get("http://127.0.0.1/test_gzip"));
  event_base->Event(socket, BEV_EVENT_CONNECTED);
  EXPECT_TRUE(socket->IsConnected());
  {
    File gzip("./testdata/gzip_reply");
    String data;
    if (gzip.Open() && gzip.ReadFile(data))
      event_base->AddData(data, socket);
  }
  while (done == false)
    event_base->Read(socket);
  event_base->Event(socket, BEV_EVENT_ERROR);
  EXPECT_DEATH(delete socket, "");
  EXPECT_DEATH(delete callback, "");
};

#endif //_NO_GZIP

TEST_F(SimpleHTTPSocket, KeepCallbackAround) {
  TestHTTPCallback* callback = new TestHTTPCallback;
  callback->keepAround = true;
  bool done = false;
  callback->done = &done;
  trippingcyril::SimpleHTTPSocket* socket = new trippingcyril::SimpleHTTPSocket(event_base, callback);
  EXPECT_FALSE(socket->IsConnected());
  EXPECT_TRUE(socket->Get("http://127.0.0.1/test"));
  event_base->Event(socket, BEV_EVENT_CONNECTED);
  EXPECT_TRUE(socket->IsConnected());
  String data = "HTTP/1.0 200 OK\r\n"
  "Content-Type: application/json\r\n"
  "Content-Length: 36\r\n"
  "Access-Control-Allow-Origin: *\r\n"
  "Connection: close\r\n\r\n{\"leet\":1337\r\n"
  ",\"numbers\": [1,2,3,4]}";
  event_base->AddData(data, socket);
  while (done == false)
    event_base->Read(socket);
  event_base->Event(socket, BEV_EVENT_ERROR);
  EXPECT_DEATH(delete socket, "");
  delete callback; // This shouldn't die this time.
};

#if __cplusplus >= 201103

TEST_F(SimpleHTTPSocket, Lamdba) {
  bool done = false;
  trippingcyril::SimpleHTTPSocket* socket = new trippingcyril::SimpleHTTPSocket(event_base
  ,[&done](unsigned short responseCode, const map<String, String>& headers, const String& response, const String& url) {
    done = true;
  }
  ,[&done](int errorCode, const String& url) {
    done = true;
  });
  EXPECT_FALSE(socket->IsConnected());
  EXPECT_TRUE(socket->Get("http://127.0.0.1/test"));
  event_base->Event(socket, BEV_EVENT_CONNECTED);
  EXPECT_TRUE(socket->IsConnected());
  String data = "HTTP/1.0 200 OK\r\n"
  "Content-Type: application/json\r\n"
  "Content-Length: 36\r\n"
  "Access-Control-Allow-Origin: *\r\n"
  "Connection: close\r\n\r\n{\"leet\":1337\r\n"
  ",\"numbers\": [1,2,3,4]}";
  event_base->AddData(data, socket);
  while (done == false)
    event_base->Read(socket);
  event_base->Event(socket, BEV_EVENT_ERROR);
};

#endif

};
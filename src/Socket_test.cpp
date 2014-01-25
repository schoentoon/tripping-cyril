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

#include "Socket.h"

namespace test {

class Socket : public LibEventTest {
};

class TestSocket : public trippingcyril::Socket {
public:
  TestSocket(Module* module) : trippingcyril::Socket(module) {
    timed_out = NULL;
    connected = NULL;
    disconnected = NULL;
  };
  virtual ~TestSocket() {
    if (expectedReadlines.empty() == false) {
      ADD_FAILURE() << "There were expected readlines which we haven't seen.";
    };
    if (expectedReadDatas.empty() == false) {
      ADD_FAILURE() << "There were expected readdatas which we havent' seen.";
    };
  };
  bool *timed_out;
  bool *connected;
  bool *disconnected;
  deque<String> expectedReadlines;
  deque<String> expectedReadDatas;
protected:
  virtual void Timeout() {
    if (timed_out)
      *timed_out = true;
  };
  virtual void Connected() {
    if (connected)
      *connected = true;
  };
  virtual void Disconnected() {
    if (disconnected)
      *disconnected = true;
  };
  virtual void ReadLine(const String& line) {
    if (expectedReadlines.empty() == true) {
      ADD_FAILURE() << "ReadLine() was called, which was unexpected.";
    };
    String expected = expectedReadlines.front();
    EXPECT_EQ(expected, line);
    expectedReadlines.pop_front();
  };
  virtual size_t ReadData(const char* data, size_t len) {
    if (expectedReadDatas.empty() == true) {
      ADD_FAILURE() << "ReadData() was called, which was unexpected.";
      return len;
    };
    String expected = expectedReadDatas.front();
    String incoming(data, len);
    EXPECT_EQ(expected, incoming);
    expectedReadDatas.pop_front();
    return len;
  };
};

TEST_F(Socket, Timeout) {
  TestSocket* socket = new TestSocket(event_base);
  ASSERT_EQ(NULL, event_base->GetBufferevent(socket));

  bool done = false;
  socket->timed_out = &done;

  EXPECT_TRUE(socket->Connect("127.0.0.1", 80));
  event_base->Event(socket, BEV_EVENT_TIMEOUT);
  EXPECT_TRUE(done);
  EXPECT_DEATH(delete socket, "");
};

TEST_F(Socket, Connected) {
  TestSocket* socket = new TestSocket(event_base);
  ASSERT_EQ(NULL, event_base->GetBufferevent(socket));
  EXPECT_FALSE(socket->IsConnected());

  bool done = false;
  socket->connected = &done;

  EXPECT_TRUE(socket->Connect("127.0.0.1", 80));
  EXPECT_FALSE(socket->IsConnected());
  event_base->Event(socket, BEV_EVENT_CONNECTED);
  EXPECT_TRUE(socket->IsConnected());
  EXPECT_TRUE(done);
  delete socket;
};

TEST_F(Socket, Disconnected) {
  TestSocket* socket = new TestSocket(event_base);
  ASSERT_EQ(NULL, event_base->GetBufferevent(socket));
  EXPECT_FALSE(socket->IsConnected());

  bool connected = false;
  socket->connected = &connected;
  bool disconnected = false;
  socket->disconnected = &disconnected;

  EXPECT_TRUE(socket->Connect("127.0.0.1", 80));
  EXPECT_FALSE(socket->IsConnected());
  event_base->Event(socket, BEV_EVENT_CONNECTED);
  EXPECT_TRUE(socket->IsConnected());
  EXPECT_TRUE(connected);
  event_base->Event(socket, BEV_EVENT_ERROR); // Anything that isn't a timeout or a connected event is a disconnect
  EXPECT_TRUE(disconnected);
  EXPECT_DEATH(delete socket,"");
};

TEST_F(Socket, ReadLine) {
  TestSocket* socket = new TestSocket(event_base);
  ASSERT_EQ(NULL, event_base->GetBufferevent(socket));
  EXPECT_FALSE(socket->IsConnected());
  EXPECT_TRUE(socket->Connect("127.0.0.1", 80));
  event_base->Event(socket, BEV_EVENT_CONNECTED);
  EXPECT_TRUE(socket->IsConnected());
  socket->SetReadLine(true);
  socket->expectedReadlines.push_front("abcdef");
  std::string data = "abcdef\r\nz";
  event_base->AddData(data, socket);
  event_base->Read(socket);
  delete socket;
};

TEST_F(Socket, ReadData) {
  TestSocket* socket = new TestSocket(event_base);
  ASSERT_EQ(NULL, event_base->GetBufferevent(socket));
  EXPECT_FALSE(socket->IsConnected());
  EXPECT_TRUE(socket->Connect("127.0.0.1", 80));
  event_base->Event(socket, BEV_EVENT_CONNECTED);
  EXPECT_TRUE(socket->IsConnected());
  socket->SetReadLine(false);
  socket->expectedReadDatas.push_front("abcdef\r\nz");
  std::string data = "abcdef\r\nz";
  event_base->AddData(data, socket);
  event_base->Read(socket);
  delete socket;
};

TEST_F(Socket, ReadLineAndData) {
  TestSocket* socket = new TestSocket(event_base);
  ASSERT_EQ(NULL, event_base->GetBufferevent(socket));
  EXPECT_FALSE(socket->IsConnected());
  EXPECT_TRUE(socket->Connect("127.0.0.1", 80));
  event_base->Event(socket, BEV_EVENT_CONNECTED);
  EXPECT_TRUE(socket->IsConnected());
  socket->SetReadLine(true);
  socket->expectedReadlines.push_front("abcdef");
  socket->expectedReadDatas.push_front("z");
  std::string data = "abcdef\r\nz";
  event_base->AddData(data, socket);
  event_base->Read(socket);
  socket->SetReadLine(false);
  event_base->Read(socket);
  delete socket;
};

};
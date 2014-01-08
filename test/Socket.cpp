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

#include "LibEventHelper.h"

#include "Socket.h"

namespace test {

class Socket : public ::testing::Test {
public:
  static LibEventHelper* event_base;
protected:
  virtual void SetUp() {
    event_base = new LibEventHelper;
  };
  virtual void TearDown() {
    delete event_base;
  };
  struct bufferevent* GetBufferevent(trippingcyril::Socket* socket) { return socket->connection; };
  void Event(trippingcyril::Socket* socket, short what) { trippingcyril::Socket::eventcb(socket->connection, what, socket); };
};

LibEventHelper* Socket::event_base = NULL;

class TestSocket : public trippingcyril::Socket {
public:
  TestSocket(Module* module) : trippingcyril::Socket(module) {
    timed_out = NULL;
    connected = NULL;
    disconnected = NULL;
  };
  virtual ~TestSocket() {};
  bool *timed_out;
  bool *connected;
  bool *disconnected;
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
};

TEST_F(Socket, Timeout) {
  TestSocket* socket = new TestSocket(event_base);
  EXPECT_EQ(NULL, GetBufferevent(socket));

  bool done = false;
  socket->timed_out = &done;

  EXPECT_TRUE(socket->Connect("127.0.0.1", 80));
  Event(socket, BEV_EVENT_TIMEOUT);
  EXPECT_TRUE(done);
  EXPECT_DEATH(delete socket, "");
};

TEST_F(Socket, Connected) {
  TestSocket* socket = new TestSocket(event_base);
  EXPECT_EQ(NULL, GetBufferevent(socket));
  EXPECT_FALSE(socket->IsConnected());

  bool done = false;
  socket->connected = &done;

  EXPECT_TRUE(socket->Connect("127.0.0.1", 80));
  EXPECT_FALSE(socket->IsConnected());
  Event(socket, BEV_EVENT_CONNECTED);
  EXPECT_TRUE(socket->IsConnected());
  EXPECT_TRUE(done);
  delete socket;
};

TEST_F(Socket, Disconnected) {
  TestSocket* socket = new TestSocket(event_base);
  EXPECT_EQ(NULL, GetBufferevent(socket));
  EXPECT_FALSE(socket->IsConnected());

  bool connected = false;
  socket->connected = &connected;
  bool disconnected = false;
  socket->disconnected = &disconnected;

  EXPECT_TRUE(socket->Connect("127.0.0.1", 80));
  EXPECT_FALSE(socket->IsConnected());
  Event(socket, BEV_EVENT_CONNECTED);
  EXPECT_TRUE(socket->IsConnected());
  EXPECT_TRUE(connected);
  Event(socket, BEV_EVENT_ERROR); // Anything that isn't a timeout or a connected event is a disconnect
  EXPECT_TRUE(disconnected);
  EXPECT_DEATH(delete socket,"");
};

};
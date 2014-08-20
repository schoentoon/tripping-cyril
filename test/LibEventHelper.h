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

#include <gtest/gtest.h>
#include <event2/buffer.h>

#include "Module.h"
#include "Socket.h"
#include "Timer.h"

using namespace trippingcyril;

namespace test {

class LibEventHelper : public Module {
public:
  LibEventHelper(unsigned int timeout = 10)
  : Module(NULL, "libevent", "/dev/null", NULL) {
    event_base = event_base_new();
    struct timeval tm;
    tm.tv_usec = 0;
    tm.tv_sec = timeout;
    EXPECT_EQ(0, event_base_loopexit(event_base, &tm));
  };
  String GetVersion() const { return "test"; };
  bool Loop() {
    return event_base_dispatch(event_base) == 0;
  };
  struct bufferevent* GetBufferevent(trippingcyril::net::Socket* socket) { return socket->_connection; };
  void Event(trippingcyril::net::Socket* socket, short what) { trippingcyril::net::Socket::eventcb(socket->_connection, what, socket); };
  void Read(trippingcyril::net::Socket* socket) { trippingcyril::net::Socket::readcb(socket->_connection, socket); };
  void AddData(const char* data, size_t len, trippingcyril::net::Socket* socket) {
    struct evbuffer* input = bufferevent_get_input(socket->_connection);
    ASSERT_EQ(0, evbuffer_unfreeze(input, 0));
    ASSERT_EQ(0, evbuffer_add(input, data, len));
    ASSERT_EQ(0, evbuffer_freeze(input, 0));
  };
  void AddData(std::string data, trippingcyril::net::Socket* socket) { AddData(data.data(), data.size(), socket); };
};

class LibEventTest : public ::testing::Test {
public:
  LibEventTest() {
    timeout = 10;
  };
  LibEventHelper* event_base;
protected:
  unsigned int timeout;
  virtual void SetUp() {
    event_base = new LibEventHelper(timeout);
  };
  virtual void TearDown() {
    delete event_base;
  };
};

};
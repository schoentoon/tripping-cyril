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

#ifndef _SOCKET_H
#define _SOCKET_H

#include <event2/bufferevent.h>

#include "String.h"
#include "Module.h"

namespace test {
  class LibEventHelper;
};

namespace trippingcyril {

class Socket {
public:
  Socket(const Module* module);
  virtual ~Socket();
  void Close();
  bool Connect(const String& hostname, uint16_t port, bool ssl = false, unsigned int timeout = 60);
  void Write(const char* data, size_t len);
  void Write(const String& data);
  void SetReadLine(bool b) { readline = b; };
  bool IsConnected() const { return is_connected; };
  void SetTimeout(double timeout);
  const Module* GetModule() const { return module; };
protected:
  virtual void Connected() {};
  virtual void Timeout() {};
  virtual void Disconnected() {};
  virtual size_t ReadData(const char* data, size_t len) { return len; };
  virtual void ReadLine(const String& line) {};
  uint8_t read_more : 1;
  bool SetTCPNoDelay(bool enable);
  bool SetTCPKeepAlive(bool enable, int delay);
private:
  uint8_t readline : 1;
  uint8_t is_connected : 1;
  uint8_t closing : 1;
  uint8_t tcp_no_delay : 1;
  uint8_t tcp_keep_alive : 1;
  int tcp_keep_alive_interval;
  struct timeval timeout;
  struct bufferevent* connection;
  const Module* module;
  static void readcb(struct bufferevent* bev, void* ctx);
  static void eventcb(struct bufferevent* bev, short what, void* ctx);
  friend class test::LibEventHelper;
};

};

#endif //_SOCKET_H
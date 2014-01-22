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

// @cond
namespace test {
  class LibEventHelper;
};
// @endcond

namespace trippingcyril {

/**
 * @brief General tcp socket class, fully async using libevent
 */
class Socket {
public:
  /**
   * General constructor
   * @param module The module to register this socket on
   */
  Socket(const Module* module);
  /**
   * General deconstructor, will close the connection if needed
   */
  virtual ~Socket();
  /**
   * Close and remove this socket
   */
  void Close();
  /**
   * Connect the actual socket
   * @param hostname The hostname to connect to
   * @param port The port to connect to
   * @param ssl True if you want to use a ssl socket
   * @param timeout Connect/read/write timeout
   * @return False if already connected or trying to connect
   */
  bool Connect(const String& hostname, uint16_t port, bool ssl = false, double timeout = 60.0);
  /**
   * Used to write data to the socket
   */
  void Write(const char* data, size_t len);
  /**
   * Used to write data to the socket
   */
  void Write(const String& data);
  /**
   * Enable/disable read line mode, decides if ReadLine or ReadData should be called
   */
  void SetReadLine(bool b) { readline = b; };
  /** @return True if socket is connected */
  bool IsConnected() const { return is_connected; };
  /**
   * Used to set a read/write timeout
   */
  void SetTimeout(double timeout);
  /**
   * Used to disable the read/write timeouts currently set
   */
  void DisableTimeout() { SetTimeout(-1); };
  /**
   * Used to get the module used to initialize this socket
   */
  const Module* GetModule() const { return module; };
protected:
  /**
   * Connected callback, will be called once the socket is connected.
   */
  virtual void Connected() {};
  /**
   * Timeout callback, will be called once either a connect, read or write timeout occurs.
   */
  virtual void Timeout() {};
  /**
   * Disconnected callback, will be called once our socket disconnects.
   */
  virtual void Disconnected() {};
  /**
   * ReadData callback, will be called if read line mode is off. Return the amount
   * of bytes that should be consumed. In case you are not done yet processing all
   * the data set read_more to 1.
   */
  virtual size_t ReadData(const char* data, size_t len) { return len; };
  /**
   * ReadLine callback, will be called if read line mode is on. Return data up until
   * a newline from the socket.
   */
  virtual void ReadLine(const String& line) {};
  /**
   * Determines if ReadData should get called again or not.
   */
  uint8_t read_more : 1;
  /**
   * Allows you to determine if the tcp no delay feature should be used or not
   */
  bool SetTCPNoDelay(bool enable);
  /**
   * Allows you to set a custom tcp keep alive interval, or disable tcp keep alive
   * all together.
   */
  bool SetTCPKeepAlive(bool enable, int delay);
private:
  // @cond
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
  // @endcond
};

};

#endif //_SOCKET_H
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

#include "defines.h"
#include "Closable.h"
#include "String.h"
#include "Module.h"
#include "Writer.h"

// @cond
#ifdef _TEST
namespace test {
  class LibEventHelper;
};
#endif //_TEST
// @endcond

namespace trippingcyril {
  namespace net {

struct IPAddress;

/**
 * @brief General tcp socket class, fully async using libevent
 */
class Socket : public Event, public Writer, public Closable {
public:
  /**
   * General constructor
   * @param module The module to register this socket on
   */
  Socket(const Module* module);
  /**
   * Constructor used to create incoming connections
   * @see Listener
   */
  Socket(struct bufferevent* event);
  /**
   * General deconstructor, will close the connection if needed
   */
  virtual ~Socket();
  /**
   * Close and remove this socket
   */
  void Close() OVERRIDE;
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
   * Connect the actual socket
   * @param ip The ip address to connect to
   * @param port The port to connect to
   * @param ssl True if you want to use a ssl socket
   * @param timeout Connect/read/write timeout
   * @return False if already connected or trying to connect
   */
  bool Connect(const IPAddress* ip, uint16_t port, bool ssl = false, double timeout= 60.0);
  /**
   * Write data to socket
   * @param data The actual data
   * @param len The length of the data
   * @return Amount of bytes written, should be equal to len
   */
  int Write(const char* data, size_t len) OVERRIDE;
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
   * Get the ip address of the remote end
   */
  const IPAddress* GetIP() const;
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
   * This will get called once it wrote something to the actual socket
   * @param bytes_left Amount of bytes left to write in our buffer
   */
  virtual void OnWrite(size_t bytes_left) {};
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
  static void readcb(struct bufferevent* bev, void* ctx);
  static void writecb(struct bufferevent* bev, void* ctx);
  static void eventcb(struct bufferevent* bev, short what, void* ctx);
#ifdef _TEST
  friend class test::LibEventHelper;
#endif //_TEST
  // @endcond
};

/**
 * @brief Simple ip address class
 */
struct IPAddress {
public:
  virtual ~IPAddress() {};
  /** @return The ip version */
  virtual int GetIPVersion() const = 0;
  /** @return Returns the human readable representation of the ip address */
  virtual String AsString() const = 0;
  /** @return Returns the human readable representation of the ip address */
  operator String() const { return AsString(); };
  // @cond
private:
  static IPAddress* fromFD(int fd);
  friend class Socket;
  // @endcond
};

/**
 * @brief The IPv4 implementation of our ip address class
 */
struct IPv4Address : public IPAddress {
public:
  IPv4Address(struct in_addr* sa) {
    addr = sa->s_addr;
  };
  /**
   * Construct from a String
   * @throws const char* on not an ip address
   */
  IPv4Address(const String& ip) {
    if (evutil_inet_pton(AF_INET, ip.c_str(), &addr) != 1)
      throw "Invalid ip address";
  };
  virtual ~IPv4Address() {};
  virtual int GetIPVersion() const OVERRIDE { return 4; };
  /** @return The ipv4 address as an integer */
  virtual int AsInt() const { return addr; }
  /** @return The ipv4 address as an integer */
  operator int() const { return AsInt(); };
  operator in_addr() const {
    struct in_addr in;
    in.s_addr = addr;
    return in;
  };
  virtual String AsString() const OVERRIDE;
private:
  in_addr_t addr;
};

  };
};

#endif //_SOCKET_H
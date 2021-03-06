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

#include <arpa/inet.h>

#include "String.h"
#include "Socket.h"

namespace trippingcyril {

/**
 * @brief A simple StatsdClient
 * @note This is really just a simple C++ wrapper around https://github.com/romanbsd/statsd-c-client
 */
class StatsdClient {
public:
  /**
   * @param nm namespace to use within statsd
   * @param hostname The hostname of the statsd instance
   * @param port The port to use
   * @throws std::runtime_error If the socket() system call failed
   * @throws std::runtime_error If getaddrinfo() failed
   */
  StatsdClient(const String& nm = "", const String& hostname = "127.0.0.1", uint16_t port = 8125);
  /**
   * @param ip The ip address to connect to
   * @param nm Namespace to use within statsd
   * @param port The port to use
   * @throws std::runtime_error If the socket() system call failed
   * @throws std::runtime_error If the ip version isn't implemented correctly just yet
   */
  StatsdClient(const net::IPAddress& ip, const String& nm = "", uint16_t port = 8125);
  virtual ~StatsdClient();
  static bool DRY_RUN;
  /** @return The namespace for this statd object */
  const String getNamespace() const { return _ns; };
  void Count(const String& stat, size_t value, float sample_rate = 1.0) const;
  void Increment(const String& stat, float sample_rate = 1.0) const;
  void Decrement(const String& stat, float sample_rate = 1.0) const;
  void Gauge(const String& stat, size_t value, float sample_rate = 1.0) const;
  void Timing(const String& stat, size_t ms, float sample_rate = 1.0) const;
private:
  // @cond
  bool shouldSend(float sample_rate) const;
  bool send(const char* message, size_t len) const;
  int prepare(char *stat, size_t value, const char *type, float sample_rate, char *buf, size_t buflen, bool lf) const;
  void cleanup(char *stat) const;
  bool send_stat(char* stat, size_t value, const char* type, float sample_rate = 1.0) const;

  struct sockaddr_in _address;
  int _sock;
  const String _ns;
  // @endcond
};

};
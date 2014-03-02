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

#ifndef _STATSD_CLIENT_H
#define _STATSD_CLIENT_H

#include <arpa/inet.h>

#include "String.h"

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
   */
  StatsdClient(const String& nm = "", const String& hostname = "127.0.0.1", uint16_t port = 8125);
  virtual ~StatsdClient();
  /** @return The namespace for this statd object */
  String getNamespace() const { return ns; };
  void Count(const String& stat, size_t value, float sample_rate = 1.0);
  void Increment(const String& stat, float sample_rate = 1.0);
  void Decrement(const String& stat, float sample_rate = 1.0);
  void Gauge(const String& stat, size_t value, float sample_rate = 1.0);
  void Timing(const String& stat, size_t ms, float sample_rate = 1.0);
protected:
  // @cond
  bool shouldSend(float sample_rate);
  bool send(const char* message, size_t len);
  int prepare(char *stat, size_t value, const char *type, float sample_rate, char *buf, size_t buflen, bool lf);
  void cleanup(char *stat);
  bool send_stat(char* stat, size_t value, const char* type, float sample_rate = 1.0);

  struct sockaddr_in address;
  int sock;
  const String ns;
  // @endcond
};

};

#endif //_STATSD_CLIENT_H
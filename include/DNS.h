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

#ifndef _DNS_H
#define _DNS_H

#include <event2/dns.h>

#include "ShouldDelete.h"
#include "Socket.h"
#include "String.h"
#include "Event.h"

#include <vector>
#include <iostream>

namespace trippingcyril {
  namespace net {
    namespace dns {

/**
 * @brief DNS callback class
 */
class DNSCallback : public ShouldDelete {
public:
  virtual ~DNSCallback() {};
  /**
   * This will get called on succesful dns lookups
   * @param query The original query address
   * @param results The found ip addresses
   * @param ttl The time to live of the results
   */
  virtual void QueryResult(const String& query, const std::vector<IPAddress*> &results, int ttl) = 0;
  /**
   * There was an error while looking up this domain name
   * @param query The original query address
   * @param errorCode The internal libevent error code
   * @param error A human readable error
   */
  virtual void Error(const String& query, int errorCode, const String& error) {};
};

/**
  * @brief A simple IPv4 lookup event
  */
class IPv4Lookup : public Event {
public:
  /**
    * General constructor
    * @param pModule The module to register this database on
    * @param query The domain name to lookup
    * @param callback The callback object
    */
  IPv4Lookup(const Module *pModule, const String& query, DNSCallback* callback);
  virtual ~IPv4Lookup();
  /** The query */
  const String GetQuery() const { return query; };
private:
  // @cond
  static void DNSEventCallback(int result, char type, int count, int ttl, void *addresses, void *arg);
  struct evdns_request* request;
  const String query;
  DNSCallback* callback;
  // @endcond
};

    };
  };
};

#endif //_DNS_H
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
    * @param pModule The module to register this dns lookup on
    * @param query The domain name to lookup
    * @param callback The callback object
    * @throws std::runtime_error If libevent somehow failed to make the request
    */
  IPv4Lookup(const Module *pModule, const String& query, DNSCallback* callback);
  virtual ~IPv4Lookup();
  /** The query */
  const String GetQuery() const { return _query; };
private:
  // @cond
  static void DNSEventCallback(int result, char type, int count, int ttl, void *addresses, void *arg);
  const String _query;
  DNSCallback* _callback;
  struct evdns_request* _request;
  // @endcond
};

/**
 * @brief Reverse dns callback
 */
class DNSReverseCallback : public ShouldDelete {
public:
  virtual ~DNSReverseCallback() {};
  /**
   * This will get called on succesful dns lookups
   * @param ip The query address
   * @param result The found hostname
   * @param ttl The time to live of the result
   * @throws std::runtime_error If libevent somehow failed to make the request
   */
  virtual void QueryResult(const IPAddress& ip, const String& result, int ttl) = 0;
  /**
   * There was an error while looking up this domain name
   * @param ip The query address
   * @param errorCode The internal libevent error code
   * @param error A human readable error
   */
  virtual void Error(const IPAddress& ip, int errorCode, const String& error) {};
};

/**
 * @brief Do a reverse ipv4 lookup
 */
class IPv4ReverseLookup : public Event {
public:
  /**
   * General constructor
   * @param module The module to register this dns lookup on
   * @param ip The ip address to lookup
   * @param callback The callback object
   */
  IPv4ReverseLookup(const Module* module, const IPv4Address& ip, DNSReverseCallback* callback);
  virtual ~IPv4ReverseLookup();
  /** The query */
  const IPv4Address GetQuery() const { return _query; };
private:
  // @cond
  static void DNSEventCallback(int result, char type, int count, int ttl, void *addresses, void* arg);
  struct evdns_request* _request;
  const IPv4Address _query;
  DNSReverseCallback* _callback;
  // @endcond
};

    };
  };
};

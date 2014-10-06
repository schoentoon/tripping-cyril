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

#if __cplusplus >= 201103
#  include <functional>
#endif

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

#if __cplusplus >= 201103
typedef std::function<void(const String& query, const std::vector<IPAddress*> &results, int ttl)> DNSLamdbaCallback;
typedef std::function<void(const String& query, int errorCode, const String& error)> DNSLamdbaErrorCallback;
#endif

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
#if __cplusplus >= 201103
  /**
   * General constructor
   * @param pModule The module to register this dns lookup on
   * @param query The domain name to lookup
   * @param callback The success callback
   * @param errorCallback The error callback function
   * @throws std::runtime_error If libevent somehow failed to make the request
   */
  IPv4Lookup(const Module *pModule, const String& query
            ,const DNSLamdbaCallback& callback
            ,const DNSLamdbaErrorCallback& errorCallback = [](const String& query, int errorCode, const String& error){})
  : IPv4Lookup(pModule, query, new DNSLamdbaCallbackImpl(callback, errorCallback)) {};
#endif
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
#if __cplusplus >= 201103
  // @cond
  class DNSLamdbaCallbackImpl : public DNSCallback {
  public:
    DNSLamdbaCallbackImpl(const DNSLamdbaCallback &_callback
    , const DNSLamdbaErrorCallback &_errorcallback)
    : callback(_callback)
    , errorcallback(_errorcallback) {
    };
    virtual ~DNSLamdbaCallbackImpl() {};
    void QueryResult(const String& query, const std::vector<IPAddress*> &results, int ttl) OVERRIDE {
      if (callback) callback(query, results, ttl);
    };
    void Error(const String& query, int errorCode, const String& error) OVERRIDE {
      if (errorcallback) errorcallback(query, errorCode, error);
    };
  private:
    const DNSLamdbaCallback callback;
    const DNSLamdbaErrorCallback errorcallback;
  };
  // @endcond
#endif
};

#if __cplusplus >= 201103
typedef std::function<void(const IPAddress& ip, const String &result, int ttl)> DNSLamdbaReverseCallback;
typedef std::function<void(const IPAddress& ip, int errorCode, const String& error)> DNSLamdbaReverseErrorCallback;
#endif

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
#if __cplusplus >= 201103
  /**
   * General constructor
   * @param module The module to register this dns lookup on
   * @param ip The ip address to lookup
   * @param callback The success callback
   * @param errorCallback The error callback function
   */
  IPv4ReverseLookup(const Module* module, const IPv4Address& ip
                   ,const DNSLamdbaReverseCallback &callback
                   ,const DNSLamdbaErrorCallback &errorCallback)
  : IPv4ReverseLookup(module, ip , new DNSLamdbaReverseCallbackImpl(callback, errorCallback)) {};
#endif
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
#if __cplusplus >= 201103
  // @cond
  class DNSLamdbaReverseCallbackImpl : public DNSReverseCallback {
  public:
    DNSLamdbaReverseCallbackImpl(const DNSLamdbaReverseCallback &_callback
    , const DNSLamdbaErrorCallback &_errorcallback)
    : callback(_callback)
    , errorcallback(_errorcallback) {
    };
    virtual ~DNSLamdbaReverseCallbackImpl() {};
    void QueryResult(const IPAddress& ip, const String& result, int ttl) OVERRIDE {
      if (callback) callback(ip, result, ttl);
    };
    void Error(const IPAddress& ip, int errorCode, const String& error) OVERRIDE {
      if (errorcallback) errorcallback(ip, errorCode, error);
    };
  private:
    const DNSLamdbaReverseCallback callback;
    const DNSLamdbaReverseErrorCallback errorcallback;
  };
  // @endcond
#endif
};

    };
  };
};

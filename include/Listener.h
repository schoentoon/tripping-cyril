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

#ifndef _LISTENER_H
#define _LISTENER_H

#include <stdint.h>
#include <openssl/ssl.h>
#include <event2/listener.h>
#include <event2/bufferevent_ssl.h>

#include "Logger.h"
#include "Socket.h"

namespace trippingcyril {
  namespace net {

/**
 * @brief Generic listener class
 */
class Listener : public Event {
public:
  /**
   * General constructor
   * @param module The module to register this listener on
   * @param pPort The port to listen on
   * @param logger The Logger to use to log accepted connections
   * @param ssl_ctx The ssl context to use to create new ssl connections, NULL means no ssl
   */
  Listener(const Module* module, uint16_t pPort, log::Logger *logger = NULL, SSL_CTX *ssl_ctx = NULL);
  /** Deconstructor */
  virtual ~Listener();
  /** @return True if we are listening */
  bool isListening() const { return listener != NULL; };
  /**
   * Start listening
   * @return True if we are succesfully listening
   */
  bool Listen();
protected:
  /**
   * Used to create the buffer event, you may override this to create it yourself
   */
  virtual struct bufferevent* createBufferEvent(evutil_socket_t fd);
  /**
   * Used to create the actual socket, create one of your own implementations here
   * of course.
   */
  virtual Socket* createSocket(struct bufferevent* bev) = 0;

  /** Logger to log accepted connection to, protected so you can pass it to new sockets */
  log::Logger *logger;
private:
  // @cond
  static void listener_cb(struct evconnlistener *evlistener, evutil_socket_t fd
                         ,struct sockaddr *sa, int socklen, void *context);
  const uint16_t port;
  struct evconnlistener *listener;
  SSL_CTX *ssl_ctx;
  // @endcond
};

  };
};

#endif //_LISTENER_H
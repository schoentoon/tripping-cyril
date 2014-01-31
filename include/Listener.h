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
#include <event2/listener.h>

#include "Socket.h"

namespace trippingcyril {

/**
 * @brief Generic listener class
 */
class Listener {
public:
  /**
   * General constructor
   * @param pPort The port to listen on
   */
  Listener(uint16_t pPort);
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
private:
  // @cond
  static void listener_cb(struct evconnlistener *evlistener, evutil_socket_t fd
                         ,struct sockaddr *sa, int socklen, void *context);
  const uint16_t port;
  struct evconnlistener *listener;
  // @endcond
};

};

#endif //_LISTENER_H
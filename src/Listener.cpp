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

#include "Listener.h"

#include <string.h>

#include "Global.h"

namespace trippingcyril {
  namespace net {

Listener::Listener(const Module* module, uint16_t pPort, log::Logger* pLogger, SSL_CTX *ctx)
: Event(module)
, _logger(pLogger)
, _port(pPort)
, _listener(NULL)
, _ssl_ctx(ctx) {
};

Listener::~Listener() {
  if (_listener != NULL)
    evconnlistener_free(_listener);
  if (_ssl_ctx != NULL)
    SSL_CTX_free(_ssl_ctx);
};

bool Listener::Listen() {
  if (_listener != NULL)
    return false;
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1)
    return false;
  struct sockaddr_in sin;
  bzero(&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(_port);
  if (bind(fd, (struct sockaddr*) &sin, sizeof(sin)) < 0)
    return false;
  _listener = evconnlistener_new_bind(GetEventBase(), Listener::listener_cb, this
                                    ,LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE|LEV_OPT_THREADSAFE, fd
                                    ,(struct sockaddr*) &sin, sizeof(sin));
  return _listener != NULL;
};

struct bufferevent* Listener::createBufferEvent(evutil_socket_t fd) {
  if (_ssl_ctx != NULL)
    return bufferevent_openssl_socket_new(GetEventBase(), fd, SSL_new(_ssl_ctx)
                                         ,BUFFEREVENT_SSL_ACCEPTING, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);
  return bufferevent_socket_new(GetEventBase(), fd, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);
};

void Listener::listener_cb(evconnlistener* evlistener, int fd, sockaddr* sa, int socklen, void* context) {
  Listener* listener = (Listener*) context;
  if (listener->_logger) {
    IPAddress* ip = IPAddress::fromFD(fd);
    String msg(ip->AsString());
    msg.append(" connected to port " + String(listener->_port));
    listener->_logger->Log(msg);
    delete ip;
  }
  struct bufferevent* bev = listener->createBufferEvent(fd);
  if (bev != NULL)
    listener->createSocket(bev);
};

  };
};
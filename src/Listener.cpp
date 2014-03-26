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

Listener::Listener(const Module* module, uint16_t pPort)
: Event(module)
, port(pPort) {
  listener = NULL;
};

Listener::~Listener() {
  if (listener != NULL)
    evconnlistener_free(listener);
};

bool Listener::Listen() {
  if (listener != NULL)
    return false;
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1)
    return false;
  struct sockaddr_in sin;
  bzero(&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  if (bind(fd, (struct sockaddr*) &sin, sizeof(sin)) < 0)
    return false;
  listener = evconnlistener_new_bind(Global::Get()->GetEventBase(), Listener::listener_cb, this
                                    ,LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE|LEV_OPT_THREADSAFE, fd
                                    ,(struct sockaddr*) &sin, sizeof(sin));
  return listener != NULL;
};

struct bufferevent* Listener::createBufferEvent(evutil_socket_t fd) {
  return bufferevent_socket_new(Global::Get()->GetEventBase(), fd, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);
};

void Listener::listener_cb(evconnlistener* evlistener, int fd, sockaddr* sa, int socklen, void* context) {
  Listener* listener = (Listener*) context;
  struct bufferevent* bev = listener->createBufferEvent(fd);
  if (bev != NULL)
    listener->createSocket(bev);
};

SSLListener::SSLListener(const Module* module, uint16_t pPort)
: Listener(module, pPort) {
  ssl_ctx = initSSLContext();
};

SSLListener::~SSLListener() {
  SSL_CTX_free(ssl_ctx);
};

SSL_CTX* SSLListener::initSSLContext() {
  SSL_CTX* ctx = SSL_CTX_new(SSLv23_server_method());
  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
  EC_KEY *ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
  SSL_CTX_set_tmp_ecdh(ctx,ecdh);
  EC_KEY_free(ecdh);
  SSL_CTX_set_default_verify_paths(ctx);
  return ctx;
};

struct bufferevent* SSLListener::createBufferEvent(int fd) {
  return bufferevent_openssl_socket_new(Global::Get()->GetEventBase(), fd, SSL_new(ssl_ctx)
                                       ,BUFFEREVENT_SSL_ACCEPTING, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);
};

};
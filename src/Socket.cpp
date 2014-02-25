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

#include "Socket.h"

#include <netinet/tcp.h>
#include <openssl/ssl.h>
#include <event2/buffer.h>
#include <event2/bufferevent_ssl.h>

#include "Global.h"

#include <arpa/inet.h>
#include <iostream>

namespace trippingcyril {

Socket::Socket(const Module* pModule)
: Event(pModule) {
  connection = NULL;
  readline = 0;
  is_connected = 0;
  closing = 0;
  read_more = 0;
  tcp_no_delay = 0;
  tcp_keep_alive = 1;
  tcp_keep_alive_interval = 45;
  SetTimeout(0.0);
};

Socket::Socket(struct bufferevent* event)
: Event(NULL) {
  connection = event;
  readline = 0;
  is_connected = 0;
  closing = 0;
  read_more = 0;
  tcp_no_delay = 0;
  tcp_keep_alive = 1;
  tcp_keep_alive_interval = 45;
  SetTimeout(0.0);
};

Socket::~Socket() {
  if (connection != NULL)
    bufferevent_free(connection);
};

void Socket::Write(const char* data, size_t len) {
  if (connection != NULL)
    bufferevent_write(connection, data, len);
};

void Socket::Write(const String& data) {
  if (connection != NULL)
    bufferevent_write(connection, data.c_str(), data.size());
};

void Socket::readcb(struct bufferevent* bev, void* ctx) {
  Socket* socket = (Socket*) ctx;
  struct evbuffer* input = bufferevent_get_input(bev);
begin:
  if (socket->readline == 1) {
    char* line = evbuffer_readln(input, NULL, EVBUFFER_EOL_CRLF);
    while (line != NULL) {
      socket->ReadLine(String(line));
      free(line);
      if (socket->readline == 0)
        goto begin; // We may have switched it in the meantime..
      line = evbuffer_readln(input, NULL, EVBUFFER_EOL_CRLF);
    };
  } else {
    do {
      socket->read_more = 0;
      size_t len = evbuffer_get_length(input);
      unsigned char* data = evbuffer_pullup(input, len);
      size_t used = socket->ReadData((const char*) data, len);
      if (used > 0) {
        evbuffer_drain(input, used);
        if (used == len)
          return; // We're completely empty so just exit
        if (socket->readline == 1)
          goto begin; // We may have switched it in the meantime..
      };
    } while (socket->read_more == 1);
  }
};

void Socket::eventcb(struct bufferevent* bev, short what, void* ctx) {
  Socket* socket = (Socket*) ctx;
  if (socket != NULL) {
    if (what & BEV_EVENT_CONNECTED) {
      socket->is_connected = 1; // This way IsConnected() will return true for our connection setters
      socket->SetTCPNoDelay(socket->tcp_no_delay == 1);
      socket->SetTCPKeepAlive(socket->tcp_keep_alive == 1, socket->tcp_keep_alive_interval);
      if (socket->timeout.tv_sec > 0 || socket->timeout.tv_usec > 0)
        bufferevent_set_timeouts(bev, &socket->timeout, &socket->timeout);
      socket->is_connected = 0;
      socket->Connected();
      socket->is_connected = 1;
    } else if (what & BEV_EVENT_TIMEOUT) {
      if (socket->closing == 0)
        socket->Timeout();
      delete socket;
    } else {
      socket->Disconnected();
      delete socket;
    }
  };
};

void Socket::Close() {
  closing = 1;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  bufferevent_set_timeouts(connection, &timeout, &timeout);
};

void Socket::SetTimeout(double dTimeout) {
  if (dTimeout > 0) {
    timeout.tv_sec = (__time_t) dTimeout;
    timeout.tv_usec = (__suseconds_t) ((dTimeout - (double) timeout.tv_sec) * 1000000.0);
    if (connection)
      bufferevent_set_timeouts(connection, &timeout, &timeout);
  } else if (connection)
    bufferevent_set_timeouts(connection, NULL, NULL);
};

const IPAddress* Socket::GetIP() const {
  if (connection == NULL)
    return NULL;
  return IPAddress::fromFD(bufferevent_getfd(connection));
};

bool Socket::SetTCPNoDelay(bool enable) {
  tcp_no_delay = enable ? 1 : 0;
  if (IsConnected()) {
    evutil_socket_t fd = bufferevent_getfd(connection);
    int mode = tcp_no_delay;
    return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &mode, sizeof(mode)) == 0;
  };
  return true;
};

bool Socket::SetTCPKeepAlive(bool enable, int delay) {
  tcp_keep_alive = enable ? 1 : 0;
  if (IsConnected()) {
    evutil_socket_t fd = bufferevent_getfd(connection);
    int mode = tcp_keep_alive ? 1 : 0;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &mode, sizeof(mode)) != 0)
      return false;
    if (!enable)
      return true;
    tcp_keep_alive_interval = delay;
    if (setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &delay, sizeof(delay)) != 0)
      return false;
    if (setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &delay, sizeof(delay)) != 0)
      return false;
  };
  return true;
};

static SSL_CTX* createSSL_CTX() {
  SSL_CTX* ssl_ctx = SSL_CTX_new(SSLv23_method());
  SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_SSLv2);
  return ssl_ctx;
};

bool Socket::Connect(const String& hostname, uint16_t port, bool ssl, double dTimeout) {
  if (connection != NULL)
    return false; // Already connected.
  struct event_base* base = (module != NULL) ? module->GetEventBase() : Global::Get()->GetEventBase();
  struct evdns_base* dns = (module != NULL) ? module->GetDNSBase() : Global::Get()->GetDNSBase();
  if (ssl) {
    static SSL_CTX* ssl_ctx = createSSL_CTX();
    SSL* ssl_obj = SSL_new(ssl_ctx);
    connection = bufferevent_openssl_socket_new(base, -1, ssl_obj, BUFFEREVENT_SSL_CONNECTING, BEV_OPT_CLOSE_ON_FREE);
  } else
    connection = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_socket_connect_hostname(connection, dns, AF_INET, hostname.c_str(), port);
  if (dTimeout > 0)
    SetTimeout(dTimeout);
  bufferevent_setcb(connection, Socket::readcb, NULL, Socket::eventcb, this);
  bufferevent_enable(connection, EV_READ);
  return true;
};

IPAddress* IPAddress::fromFD(int fd) {
  if (fd < 0)
    return NULL;
  struct sockaddr_in sa;
  socklen_t size = sizeof(sa);
  if (getpeername(fd, (struct sockaddr*) &sa, &size) == 0) {
    switch (((struct sockaddr*) &sa)->sa_family) {
      case AF_INET:
        return new IPv4Address(&sa.sin_addr);
      /*case AF_INET6: // TODO Add IPv6 support in general
        return new IPv6Address(&sa);*/
      default:
        return NULL;
    };
  };
  return NULL;
};

String IPv4Address::AsString() const {
  char buf[INET_ADDRSTRLEN];
  size_t len = snprintf(buf, sizeof(buf), "%d.%d.%d.%d", (addr      ) & 0xFF
                                                       , (addr >>  8) & 0xFF
                                                       , (addr >> 16) & 0xFF
                                                       , (addr >> 24) & 0xFF);
  return String(buf, len);
};

};
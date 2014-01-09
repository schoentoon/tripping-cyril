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

#include <openssl/ssl.h>
#include <event2/buffer.h>
#include <event2/bufferevent_ssl.h>

#include "Global.h"

namespace trippingcyril {

Socket::Socket(Module* module) {
  this->module = module;
  connection = NULL;
  readline = 0;
  is_connected = 0;
  closing = 0;
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
  if (socket->readline == 1) {
    char* line = evbuffer_readln(input, NULL, EVBUFFER_EOL_CRLF);
    while (line != NULL) {
      socket->ReadLine(String(line));
      free(line);
      if (socket->readline != 1) { // We may have switched it in the meantime..
        readcb(bev, ctx);
        return;
      };
      line = evbuffer_readln(input, NULL, EVBUFFER_EOL_CRLF);
    };
  } else {
    size_t len = evbuffer_get_length(input);
    unsigned char* data = evbuffer_pullup(input, len);
    size_t used = socket->ReadData((const char*) data, len);
    if (used > 0)
      evbuffer_drain(input, used);
  }
};

void Socket::eventcb(struct bufferevent* bev, short what, void* ctx) {
  Socket* socket = (Socket*) ctx;
  if (socket != NULL) {
    if (what & BEV_EVENT_CONNECTED) {
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
  SetTimeout(0.0);
};

void Socket::SetTimeout(double timeout) {
  if (connection) {
    struct timeval tv;
    tv.tv_sec = (__time_t) timeout;
    tv.tv_usec = (__suseconds_t) ((timeout - (double) tv.tv_sec) * 1000000.0);
    bufferevent_set_timeouts(connection, &tv, &tv);
  };
};

bool Socket::Connect(const String& hostname, uint16_t port, bool ssl, unsigned int timeout) {
  if (connection != NULL)
    return false; // Already connected.
  struct event_base* base = (module != NULL) ? module->GetEventBase() : Global::Get()->GetEventBase();
  struct evdns_base* dns = (module != NULL) ? module->GetDNSBase() : Global::Get()->GetDNSBase();
  if (ssl) {
    static SSL_CTX* ssl_ctx = SSL_CTX_new(SSLv23_method());
    SSL* ssl_obj = SSL_new(ssl_ctx);
    connection = bufferevent_openssl_socket_new(base, -1, ssl_obj, BUFFEREVENT_SSL_CONNECTING, BEV_OPT_CLOSE_ON_FREE);
  } else
    connection = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_socket_connect_hostname(connection, dns, AF_INET, hostname.c_str(), port);
  if (timeout > 0) {
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    bufferevent_set_timeouts(connection, &tv, &tv);
  }
  bufferevent_setcb(connection, Socket::readcb, NULL, Socket::eventcb, this);
  bufferevent_enable(connection, EV_READ);
  return true;
};

};
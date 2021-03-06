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

#include "StatsdClient.h"

#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <unistd.h>
#include <netdb.h>

namespace trippingcyril {

StatsdClient::StatsdClient(const String& pNm, const String& hostname, uint16_t port)
: _ns(pNm) {
  _sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (_sock == -1)
    throw std::runtime_error("socket() returned -1");
  _address.sin_family = AF_INET;
  _address.sin_port = htons(port);
  if (inet_aton(hostname.c_str(), &_address.sin_addr) == 0) {
    struct addrinfo* result = NULL, hints;
    bzero(&hints, sizeof(hints));
    int error;
    if ((error = getaddrinfo(hostname.c_str(), NULL, &hints, &result)))
      throw std::runtime_error(gai_strerror(error));
    memcpy(&_address.sin_addr, &((struct sockaddr_in*)result->ai_addr)->sin_addr, sizeof(struct in_addr));
    freeaddrinfo(result);
  };
};

StatsdClient::StatsdClient(const net::IPAddress& ip, const String& pNm, uint16_t port)
: _ns(pNm) {
  _sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (_sock == -1)
    throw std::runtime_error("socket() returned -1");
  _address.sin_family = AF_INET;
  _address.sin_port = htons(port);
  switch (ip.GetIPVersion()) {
  case 4: {
    const net::IPv4Address ipv4 = (const net::IPv4Address) ip;
    _address.sin_addr = ipv4;
    break;
  };
  default:
    throw std::runtime_error("Use of not implemented ip version");
  };
};

StatsdClient::~StatsdClient() {
  if (_sock != -1)
    close(_sock);
};

bool StatsdClient::DRY_RUN = false;

void StatsdClient::Count(const String& stat, size_t value, float sample_rate) const {
  send_stat((char*) stat.c_str(), value, "c", sample_rate);
};

void StatsdClient::Increment(const String& stat, float sample_rate) const {
  Count(stat, 1, sample_rate);
};

void StatsdClient::Decrement(const String& stat, float sample_rate) const {
  Count(stat, -1, sample_rate);
};

void StatsdClient::Gauge(const String& stat, size_t value, float sample_rate) const {
  send_stat((char*) stat.c_str(), value, "g", sample_rate);
};

void StatsdClient::Timing(const String& stat, size_t ms, float sample_rate) const {
  send_stat((char*) stat.c_str(), ms, "ms", sample_rate);
};

bool StatsdClient::shouldSend(float sample_rate) const {
  if (sample_rate < 1.0) {
    float p = ((float) random() / RAND_MAX);
    return sample_rate > p;
  }
  return true;
};

bool StatsdClient::send(const char* message, size_t len) const {
  if (DRY_RUN)
    return true;
  if (sendto(_sock, message, len, 0, (struct sockaddr*) &_address, sizeof(_address)) == -1)
    return false;
  return true;
};

int StatsdClient::prepare(char* stat, size_t value, const char* type, float sample_rate, char* buf, size_t buflen, bool lf) const {
  if (sample_rate == 1.0)
    return snprintf(buf, buflen, "%s%s:%zd|%s%s", _ns.c_str(), stat, value, type, lf ? "\n" : "");
  else
    return snprintf(buf, buflen, "%s%s:%zd|%s|@%.2f%s", _ns.c_str(), stat, value, type, sample_rate, lf ? "\n" : "");
};

void StatsdClient::cleanup(char* stat) const {
  char *p;
  for (p = stat; *p; ++p) {
    if (*p == ':' || *p == '|' || *p == '@')
      *p = '_';
  };
};

bool StatsdClient::send_stat(char* stat, size_t value, const char* type, float sample_rate) const {
  if (shouldSend(sample_rate) == false)
    return false;
  char buf[BUFSIZ];
  int len = prepare(stat, value, type, sample_rate, buf, sizeof(buf), 0);
  if (len < 0)
    return false;
  return send(buf, len);
};

};
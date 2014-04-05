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

#include "DNS.h"

#include "Module.h"
#include "Global.h"

namespace trippingcyril {

IPv4Lookup::IPv4Lookup(const Module* pModule, const String& pQuery, DNSCallback* pCallback)
: Event(pModule)
, query(pQuery) {
  callback = pCallback;
  request = evdns_base_resolve_ipv4((module != NULL) ? module->GetDNSBase() : Global::Get()->GetDNSBase()
    ,query.c_str(), 0, IPv4Lookup::DNSEventCallback, this);
};

IPv4Lookup::~IPv4Lookup() {
  if (callback && callback->shouldDelete())
    delete callback;
  if (request)
    evdns_cancel_request((module != NULL) ? module->GetDNSBase() : Global::Get()->GetDNSBase(), request);
};

void IPv4Lookup::DNSEventCallback(int result, char type, int count, int ttl, void* addresses, void* arg) {
  IPv4Lookup* lookup = (IPv4Lookup*) arg;
  if (lookup) {
    if (lookup->callback) {
      if (result == DNS_ERR_NONE) {
        std::vector<IPAddress*> output;
        struct in_addr* in_addrs = (struct in_addr*) addresses;
        for (int i = 0; i < count; ++i) {
          IPv4Address *ip = new IPv4Address(&in_addrs[i]);
          output.push_back(ip);
        };
        lookup->callback->QueryResult(lookup->query, output, ttl);
        while (output.empty() == false) {
          IPAddress* ip = output[0];
          delete ip;
          output.erase(output.begin());
        };
      } else
        lookup->callback->Error(lookup->query, result, evdns_err_to_string(result));
    };
    delete lookup;
  };
};

};
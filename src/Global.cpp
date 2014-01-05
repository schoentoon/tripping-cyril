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

#include "Global.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

namespace trippingcyril {

Global::Global() {
  SSL_library_init();
  ERR_load_CRYPTO_strings();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
  RAND_poll();
  event_base = event_base_new();
  dns_base = evdns_base_new(event_base, 1);
};

Global::~Global() {
  event_base_free(event_base);
  evdns_base_free(dns_base, 0);
};

void Global::Loop() {
  while (true)
    event_base_dispatch(event_base);
};

};
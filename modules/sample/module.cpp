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

#include "module.h"

#include "SimpleHTTPSocket.h"

#include <iostream>

using namespace module::sample;

class PrintHttp : public HTTPCallback {
public:
  void OnRequestDone(unsigned short responseCode, const map<String, String>& headers, const String& response, const String& url) {
    cerr << response << endl;
  };
};

MODCONSTRUCTOR(SampleModule) {
  std::cerr << "module::sample::SampleModule::SampleModule();" << std::endl;
};

SampleModule::~SampleModule() {
  std::cerr << "module::sample::SampleModule::~SampleModule();" << std::endl;
};

void SampleModule::OnLoaded() {
  std::cerr << "module::sample::SampleModule::OnLoaded();" << std::endl;
  trippingcyril::SimpleHTTPSocket* socket = new SimpleHTTPSocket(this, new PrintHttp);
  socket->Get("http://httpbin.org/stream/100");
  socket = new SimpleHTTPSocket(this, new PrintHttp);
  socket->Get("http://httpbin.org/gzip");
  socket = new SimpleHTTPSocket(this, new PrintHttp);
  socket->Post("http://httpbin.org/post", "{\"test\":true}", "application/json");
  //int* foo = (int*) -1;
  //std::cerr << *foo << std::endl;
};

MODULEDEFS(SampleModule);
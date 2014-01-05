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

#include "String.h"

#include <stdlib.h>

#include <sstream>

namespace trippingcyril {

String::String(char c) : string() { stringstream s; s << c; *this = s.str(); };
String::String(unsigned char c) : string() { stringstream s; s << c; *this = s.str(); };
String::String(short i) : string() { stringstream s; s << i; *this = s.str(); };
String::String(unsigned short i) : string() { stringstream s; s << i; *this = s.str(); };
String::String(int i) : string() { stringstream s; s << i; *this = s.str(); };
String::String(unsigned int i) : string() { stringstream s; s << i; *this = s.str(); };
String::String(long i) : string() { stringstream s; s << i; *this = s.str(); };
String::String(unsigned long i) : string() { stringstream s; s << i; *this = s.str(); };
String::String(long long i) : string() { stringstream s; s << i; *this = s.str(); };
String::String(unsigned long long i) : string() { stringstream s; s << i; *this = s.str(); };
String::String(double i, unsigned int precision) : string() { stringstream s; s.precision(precision); s << std::fixed << i; *this = s.str(); };
String::String(float i, unsigned int precision) : string() { stringstream s; s.precision(precision); s << std::fixed << i; *this = s.str(); };
String::String(const std::stringstream& stream) : string() { *this = stream.str(); };

bool String::ToBool() const {
  return *this == "true";
};

short String::ToShort(unsigned int base) const {
  return strtoul(this->c_str(), (char**) NULL, base);
};

unsigned short String::ToUShort(unsigned int base) const {
  return strtoul(this->c_str(), (char**) NULL, base);
};

int String::ToInt(unsigned int base) const {
  return strtoul(this->c_str(), (char**) NULL, base);
};

unsigned int String::ToUInt(unsigned int base) const {
  return strtoul(this->c_str(), (char**) NULL, base);
};

long String::ToLong(unsigned int base) const {
  return strtoul(this->c_str(), (char**) NULL, base);
};

unsigned long String::ToULong(unsigned int base) const {
  return strtoul(this->c_str(), (char**) NULL, base);
};

long long String::ToLongLong(unsigned int base) const {
  return strtoll(this->c_str(), (char**) NULL, base);
};

unsigned long long String::ToULongLong(unsigned int base) const {
  return strtoll(this->c_str(), (char**) NULL, base);
};

double String::ToDouble() const {
  return strtod(this->c_str(), NULL);
};

float String::ToFloat() const {
  return strtof(this->c_str(), NULL);
};

};
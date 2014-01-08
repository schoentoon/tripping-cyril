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

bool String::WildCmp(const String& sWild) {
  const char* wild = sWild.c_str();
  const char* str = this->c_str();
  const char* cp = NULL;
  const char* mp = NULL;
  while (*str != '\0' && (*wild != '*')) {
    if ((*wild != *str) && (*wild != '?'))
      return false;
    wild++;
    str++;
  };
  while (*str != '\0') {
    if (*wild == '*') {
      if (*++wild == '\0')
        return true;
      mp = wild;
      cp = str+1;
    } else if ((*wild == *str) || (*wild == '?')) {
      wild++;
      str++;
    } else {
      wild = mp;
      str = cp++;
    };
  };
  while (*wild == '*')
    wild++;
  return (*wild == '\0');
};

bool String::Trim(const String& to_trim) {
  return TrimRight(to_trim) || TrimLeft(to_trim);
};

bool String::TrimLeft(const String& to_trim) {
  size_type i = find_first_not_of(to_trim);
  if (i == 0)
    return false;
  if (i != npos)
    erase(0, i);
  else
    clear();
  return true;
};

bool String::TrimRight(const String& to_trim) {
  size_type i = find_last_not_of(to_trim);
  if (i + 1 == length())
    return false;
  if (i != npos)
    erase(i + 1, npos);
  else
    clear();
  return true;
};

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
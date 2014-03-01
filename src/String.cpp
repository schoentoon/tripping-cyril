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

#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>

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

String String::ToPercent(double d) {
  char szRet[32];
  snprintf(szRet, 32, "%.02f%%", d);
  return szRet;
};

String String::ToByteStr(unsigned long long d) {
  const unsigned long long KiB = 1024;
  const unsigned long long MiB = KiB * 1024;
  const unsigned long long GiB = MiB * 1024;
  const unsigned long long TiB = GiB * 1024;
  if (d > TiB)
    return String(d / (double) TiB) + " TiB";
  else if (d > GiB)
    return String(d / (double) GiB) + " GiB";
  else if (d > MiB)
    return String(d / (double) MiB) + " MiB";
  else if (d > KiB)
    return String(d / (double) KiB) + " KiB";
  return String(d) + " B";
};

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

String& String::MakeLower() {
  for (unsigned int c = 0; c < length(); ++c) {
    char& ch = (*this)[c];
    ch = tolower(ch);
  };
  return *this;
};

String& String::MakeUpper() {
  for (unsigned int c = 0; c < length(); ++c) {
    char& ch = (*this)[c];
    ch = toupper(ch);
  };
  return *this;
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

bool String::Base64Encode(String& sRet) const {
  const char b64table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  sRet.clear();
  size_t len = size();
  const unsigned char* input = (const unsigned char*) c_str();
  unsigned char *output, *p;
  size_t i = 0, mod = len % 3, toalloc;
  toalloc = (len / 3) * 4 + (3 - mod) % 3 + 1 + 8;
  if (toalloc < len)
    return false;
  p = output = new unsigned char [toalloc];
  while (i < len - mod) {
    *p++ = b64table[input[i++] >> 2];
    *p++ = b64table[((input[i - 1] << 4) | (input[i] >> 4)) & 0x3f];
    *p++ = b64table[((input[i] << 2) | (input[i + 1] >> 6)) & 0x3f];
    *p++ = b64table[input[i + 1] & 0x3f];
    i += 2;
  }
  if (mod) {
    *p++ = b64table[input[i++] >> 2];
    *p++ = b64table[((input[i - 1] << 4) | (input[i] >> 4)) & 0x3f];
    if (mod == 1)
      *p++ = '=';
    else
      *p++ = b64table[(input[i] << 2) & 0x3f];
    *p++ = '=';
  }
  *p = 0;
  sRet = (char*) output;
  delete[] output;
  return true;
};

static const unsigned char XX = 0xff;
static const unsigned char base64_table[256] = {
  XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
  XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
  XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,62, XX,XX,XX,63,
  52,53,54,55, 56,57,58,59, 60,61,XX,XX, XX,XX,XX,XX,
  XX, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
  15,16,17,18, 19,20,21,22, 23,24,25,XX, XX,XX,XX,XX,
  XX,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
  41,42,43,44, 45,46,47,48, 49,50,51,XX, XX,XX,XX,XX,
  XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
  XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
  XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
  XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
  XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
  XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
  XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
  XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
};

unsigned long String::Base64Decode(String& sRet) const {
  String sTmp(*this);
  sTmp.Trim();
  const char* in = sTmp.c_str();
  char c, c1, *p;
  unsigned long i;
  unsigned long uLen = sTmp.size();
  char* out = new char[uLen + 1];
  for (i = 0, p = out; i < uLen; i++) {
    c = (char) base64_table[(unsigned char)in[i++]];
    c1 = (char) base64_table[(unsigned char)in[i++]];
    *p++ = (c << 2) | ((c1 >> 4) & 0x3);
    if (i < uLen) {
      if (in[i] == '=')
        break;
      c = (char) base64_table[(unsigned char)in[i]];
      *p++ = ((c1 << 4) & 0xf0) | ((c >> 2) & 0xf);
    }
    if (++i < uLen) {
      if (in[i] == '=')
        break;
      *p++ = ((c << 6) & 0xc0) | (char) base64_table[(unsigned char)in[i]];
    }
  }
  *p = '\0';
  unsigned long uRet = p - out;
  sRet.clear();
  sRet.append(out, uRet);
  delete[] out;
  return uRet;
};

String String::SHA512(bool raw) const {
  unsigned char digest[SHA512_DIGEST_LENGTH];
  ::SHA512((unsigned char*) data(), size(), (unsigned char*) digest);
  if (raw)
    return String((const char*) digest, sizeof(digest));

  char output[SHA512_DIGEST_LENGTH*2];
  for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
    sprintf(&output[i*2], "%02x", (unsigned int)digest[i]);
  return String(output, sizeof(output));
};

};
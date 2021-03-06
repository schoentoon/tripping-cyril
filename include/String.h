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

#pragma once

#include <string>

#include <stdint.h>

using namespace std;

namespace trippingcyril {

/**
 * @brief A wrapper string class to make a various amount of things just easier
 * to type. All strings used should be of this type, add methods to this class
 * as you need them.
 */
class String : public string {
public:
  explicit String(bool b) : string(b ? "true" : "false") {};
  explicit String(char c);
  explicit String(unsigned char c);
  explicit String(short i);
  explicit String(unsigned short i);
  explicit String(int i);
  explicit String(unsigned int i);
  explicit String(long i);
  explicit String(unsigned long i);
  explicit String(long long i);
  explicit String(unsigned long long i);
  explicit String(double i, unsigned int precision = 2);
  explicit String(float i, unsigned int precision = 2);
  String(const std::stringstream& stream);
  String() : string() {};
  String(const char* c) : string(c) {};
  String(const char* c, size_t l) : string(c, l) {};
  String(const string& s) : string(s) {};
  virtual ~String() {};

  /** Pretty-print a percent value.
   * @param d The percent value. This should be in range 0-100.
   * @return The "pretty" string.
   */
  static String ToPercent(double d);
  /** Pretty-print a number of bytes.
   * @param d The number of bytes.
   * @return A string describing the number of bytes.
   */
  static String ToByteStr(unsigned long long d);

  /**
   * Do a wildcard comparison on this string. Where "*" will match any number of
   * characters and "?" will match a single character. For example
   * <code>String("I_am@~bar@foo").WildCmp("*!?bar@foo")</code>
   * would return true.
   * @param wild The wildcard to use for this comparison
   * @return true if the wildcard matches
   */
  bool WildCmp(const String& wild) const;

  /**
   * Trim this string, all leading occurences of characters to_trim are removed.
   * @param to_trim A list of characters that should be trimmed.
   * @return true if this string as modified.
   */
  bool Trim(const String& to_trim = " \t\r\n");
  /**
   * Trim this string, all leading occurences of characters to_trim are removed.
   * @param to_trim A list of characters that should be trimmed.
   * @return true if this string as modified.
   */
  bool TrimLeft(const String& to_trim = " \t\r\n");
  /**
   * Trim this string, all leading occurences of characters to_trim are removed.
   * @param to_trim A list of characters that should be trimmed.
   * @return true if this string as modified.
   */
  bool TrimRight(const String& to_trim = " \t\r\n");
  /**
   * Turn all characters in this string into their lower-case equivalent.
   * @return A reference to *this.
   */
  String& MakeLower();
  /**
   * Turn all characters in this string into their upper-case equivalent.
   * @return A reference to *this.
   */
  String& MakeUpper();
  /**
   * Check if the string is formed with only the characters provided
   * @return False if a character not listed in the parameter is found
   */
  bool OnlyContains(const String& characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-") const;

  /** @return True if this string is not "false". */
  bool ToBool() const;
  /** @return True if this string is not "false". */
  operator bool() const { return ToBool(); };

  /** @return The numerical value of this string similar to atoi(). */
  short ToShort(unsigned int base = 10) const;
  /** @return The numerical value of this string similar to atoi(). */
  operator short() const { return ToShort(); };

  /** @return The numerical value of this string similar to atoi(). */
  unsigned short ToUShort(unsigned int base = 10) const;
  /** @return The numerical value of this string similar to atoi(). */
  operator unsigned short() const { return ToUShort(); };

  /** @return The numerical value of this string similar to atoi(). */
  int ToInt(unsigned int base = 10) const;

  /** @return The numerical value of this string similar to atoi(). */
  unsigned int ToUInt(unsigned int base = 10) const;

  /** @return The numerical value of this string similar to atoi(). */
  long ToLong(unsigned int base = 10) const;

  /** @return The numerical value of this string similar to atoi(). */
  unsigned long ToULong(unsigned int base = 10) const;

  /** @return The numerical value of this string similar to atoi(). */
  long long ToLongLong(unsigned int base = 10) const;

  /** @return The numerical value of this string similar to atoi(). */
  unsigned long long ToULongLong(unsigned int base = 10) const;

  /** @return The numerical value of this string similar to atoi(). */
  double ToDouble() const;
  /** @return The numerical value of this string similar to atoi(). */
  operator double() const { return ToDouble(); };

  /** @return The numerical value of this string similar to atoi(). */
  float ToFloat() const;
  /** @return The numerical value of this string similar to atoi(). */
  operator float() const { return ToFloat(); };

  /** @return The numerical value of this string similar to atoi(). */
  int32_t ToInt32() const { return ToLongLong(); };
  /** @return The numerical value of this string similar to atoi(). */
  operator int32_t() const { return ToInt32(); };
  /** @return The numerical value of this string similar to atoi(). */
  uint32_t ToUInt32() const { return ToULongLong(); };
  /** @return The numerical value of this string similar to atoi(). */
  operator uint32_t() const { return ToUInt32(); };
  /** @return The numerical value of this string similar to atoi(). */
  int64_t ToInt64() const { return ToLongLong(); };
  /** @return The numerical value of this string similar to atoi(). */
  operator int64_t() const { return ToInt64(); };
  /** @return The numerical value of this string similar to atoi(). */
  uint64_t ToUInt64() const { return ToULongLong(); };
  /** @return The numerical value of this string similar to atoi(). */
  operator uint64_t() const { return ToUInt64(); };

  /** Base64-encode the current string.
   * @param sRet String where the result is saved.
   * @return True if succesfully coded
   */
  bool Base64Encode(String& sRet) const;
  /** Treat this string as base64-encoded data and decode it.
   * @param sRet String to which the result of the decode is safed.
   * @return The length of the resulting string.
   */
  unsigned long Base64Decode(String& sRet) const;

  /**
   * Returns a SHA1 hash of this string
   * @param raw If true it'll return the raw unencoded hash, probably not printable
   */
  String SHA1(bool raw = false) const;
  /**
   * Returns a SHA256 hash of this string
   * @param raw If true it'll return the raw unencoded hash, probably not printable
   */
  String SHA256(bool raw = false) const;
  /**
   * Returns a SHA512 hash of this string
   * @param raw If true it'll return the raw unencoded hash, probably not printable
   */
  String SHA512(bool raw = false) const;
};

};

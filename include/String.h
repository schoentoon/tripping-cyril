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

#ifndef _TC_STRING_H
#define _TC_STRING_H

#include <string>

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

  /**
   * Do a wildcard comparison on this string. Where "*" will match any number of
   * characters and "?" will match a single character. For example
   * <code>String("I_am@~bar@foo").WildCmp("*!?bar@foo")</code>
   * would return true.
   * @param wild The wildcard to use for this comparison
   * @return true if the wildcard matches
   */
  bool WildCmp(const String& wild);

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

  /** @return True if this string is not "false". */
  bool ToBool() const;
  /** @return True if this string is not "false". */
  operator bool() { return ToBool(); };

  /** @return The numerical value of this string similar to atoi(). */
  short ToShort(unsigned int base = 10) const;
  /** @return The numerical value of this string similar to atoi(). */
  operator short() { return ToShort(); };

  /** @return The numerical value of this string similar to atoi(). */
  unsigned short ToUShort(unsigned int base = 10) const;
  /** @return The numerical value of this string similar to atoi(). */
  operator unsigned short() { return ToUShort(); };

  /** @return The numerical value of this string similar to atoi(). */
  int ToInt(unsigned int base = 10) const;
  /** @return The numerical value of this string similar to atoi(). */
  operator int() { return ToInt(); };

  /** @return The numerical value of this string similar to atoi(). */
  unsigned int ToUInt(unsigned int base = 10) const;
  /** @return The numerical value of this string similar to atoi(). */
  operator unsigned int() { return ToUInt(); };

  /** @return The numerical value of this string similar to atoi(). */
  long ToLong(unsigned int base = 10) const;
  /** @return The numerical value of this string similar to atoi(). */
  operator long() { return ToLong(); };

  /** @return The numerical value of this string similar to atoi(). */
  unsigned long ToULong(unsigned int base = 10) const;
  /** @return The numerical value of this string similar to atoi(). */
  operator unsigned long() { return ToULong(); };

  /** @return The numerical value of this string similar to atoi(). */
  long long ToLongLong(unsigned int base = 10) const;
  /** @return The numerical value of this string similar to atoi(). */
  operator long long() { return ToLongLong(); };

  /** @return The numerical value of this string similar to atoi(). */
  unsigned long long ToULongLong(unsigned int base = 10) const;
  /** @return The numerical value of this string similar to atoi(). */
  operator unsigned long long() { return ToULongLong(); };

  /** @return The numerical value of this string similar to atoi(). */
  double ToDouble() const;
  /** @return The numerical value of this string similar to atoi(). */
  operator double() { return ToDouble(); };

  /** @return The numerical value of this string similar to atoi(). */
  float ToFloat() const;
  /** @return The numerical value of this string similar to atoi(). */
  operator float() { return ToFloat(); };
};

};

#endif //_TC_STRING_H
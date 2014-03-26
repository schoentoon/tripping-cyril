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

#include <gtest/gtest.h>

#include <stdint.h>

#include "String.h"

using namespace trippingcyril;

namespace test {

TEST(String, OnlyContains) {
  String str("abcdef");
  EXPECT_TRUE(str.OnlyContains("abcdefghijklmnop"));
  EXPECT_FALSE(str.OnlyContains("ABCDEF"));
};

TEST(String, ToPercent) {
  String percent = String::ToPercent(1.337);
  EXPECT_EQ(percent, "1.34%");
};

TEST(String, ToByteStr) {
  String size = String::ToByteStr(9001);
  EXPECT_EQ(size, "8.79 KiB");
  uint64_t max = ~0;
  size = String::ToByteStr(max);
  EXPECT_EQ(size, "16777216.00 TiB"); //That's a lot..
};

TEST(String, Bool) {
  String t(true);
  EXPECT_EQ(t, "true");
  EXPECT_TRUE(t.ToBool());
  bool b = t;
  EXPECT_TRUE(b);
  String f(false);
  EXPECT_EQ(f, "false");
  EXPECT_FALSE(f.ToBool());
};

TEST(String, Char) {
  String one(0x01);
  EXPECT_EQ(one, "1");
};

TEST(String, Short) {
  String leet((short) 1337);
  EXPECT_EQ(leet, "1337");
  String ninethousand((unsigned short) 9001);
  EXPECT_EQ(ninethousand, "9001");
  EXPECT_EQ(1337, leet.ToUShort());
  EXPECT_EQ(9001, ninethousand.ToShort());
  short s = leet;
  EXPECT_EQ(s, 1337);
};

TEST(String, Int) {
  String leet((int) 1337);
  EXPECT_EQ(leet, "1337");
  String ninethousand((unsigned int) 9001);
  EXPECT_EQ(ninethousand, "9001");
  EXPECT_EQ(1337, leet.ToUInt());
  EXPECT_EQ(9001, ninethousand.ToInt());
  int s = leet;
  EXPECT_EQ(s, 1337);
};

TEST(String, Long) {
  String leet((long) 1337);
  EXPECT_EQ(leet, "1337");
  String ninethousand((unsigned long) 9001);
  EXPECT_EQ(ninethousand, "9001");
  EXPECT_EQ(1337, leet.ToULong());
  EXPECT_EQ(9001, ninethousand.ToLong());
  long s = leet;
  EXPECT_EQ(s, 1337);
};

TEST(String, LongLong) {
  String leet((long long) 1337);
  EXPECT_EQ(leet, "1337");
  String ninethousand((unsigned long long) 9001);
  EXPECT_EQ(ninethousand, "9001");
  EXPECT_EQ(1337, leet.ToULongLong());
  EXPECT_EQ(9001, ninethousand.ToLongLong());
  long long s = leet;
  EXPECT_EQ(s, 1337);
};

TEST(String, Float) {
  String pi((float) 3.14159265359, 3);
  EXPECT_EQ(pi, "3.142");
  EXPECT_FLOAT_EQ(3.142, pi.ToFloat());
  float s = pi;
  EXPECT_FLOAT_EQ(3.142, s);
};

TEST(String, Double) {
  String pi((double) 3.14159265359, 5);
  EXPECT_EQ(pi, "3.14159");
  EXPECT_DOUBLE_EQ(3.14159, pi.ToDouble());
  double s = pi;
  EXPECT_DOUBLE_EQ(3.14159, s);
};

TEST(String, WildCmp) {
  String s("abcdefghijklmnop");
  EXPECT_TRUE(s.WildCmp("a*p"));
  EXPECT_TRUE(s.WildCmp("?bcdefghijklmnop"));
  EXPECT_TRUE(s.WildCmp("?bc*hijk*"));
};

TEST(String, Trim) {
  String s("abcdef\r\n");
  EXPECT_TRUE(s.Trim());
  EXPECT_EQ(s, "abcdef");
};

TEST(String, MakeLower) {
  String s("aBCDEF");
  EXPECT_EQ(s.MakeLower(), "abcdef");
  EXPECT_EQ(s, "abcdef");
};

TEST(String, MakeUpper) {
  String s("abcdEF");
  EXPECT_EQ(s.MakeUpper(), "ABCDEF");
  EXPECT_EQ(s, "ABCDEF");
};

TEST(String, Base64Encoding) {
  String s("test");
  String output;
  EXPECT_TRUE(s.Base64Encode(output));
  EXPECT_EQ(output, "dGVzdA==");
};

TEST(String, Base64Decoding) {
  String s("dGVzdA==");
  String output;
  unsigned long ret = s.Base64Decode(output);
  EXPECT_EQ(ret, output.size());
  EXPECT_EQ(output, "test");
};

TEST(String, SHA1) {
  String correctoutput("18af819125b70879d36378431c4e8d9bfa6a2599");
  String s("Test string");
  String hash = s.SHA1();
  EXPECT_EQ(correctoutput, hash);
  EXPECT_EQ(40, hash.size());
  hash = s.SHA1(true);
  EXPECT_EQ(20, hash.size());
  unsigned char rawhash[] = { 0x18, 0xaf, 0x81, 0x91, 0x25, 0xb7, 0x08, 0x79, 0xd3, 0x63
    , 0x78, 0x43, 0x1c, 0x4e, 0x8d, 0x9b, 0xfa, 0x6a, 0x25, 0x99 };
  EXPECT_EQ(String((const char*) rawhash, sizeof(rawhash)), hash);
};

TEST(String, SHA256) {
  String correctoutput("a3e49d843df13c2e2a7786f6ecd7e0d184f45d718d1ac1a8a63e570466e489dd");
  String s("Test string");
  String hash = s.SHA256();
  EXPECT_EQ(correctoutput, hash);
  EXPECT_EQ(64, hash.size());
  hash = s.SHA256(true);
  EXPECT_EQ(32, hash.size());
  unsigned char rawhash[] = { 0xa3, 0xe4, 0x9d, 0x84, 0x3d, 0xf1, 0x3c, 0x2e, 0x2a, 0x77
    , 0x86, 0xf6, 0xec, 0xd7, 0xe0, 0xd1, 0x84, 0xf4, 0x5d, 0x71, 0x8d, 0x1a, 0xc1, 0xa8
    , 0xa6, 0x3e, 0x57, 0x04, 0x66, 0xe4, 0x89, 0xdd };
  EXPECT_EQ(String((const char*) rawhash, sizeof(rawhash)), hash);
};

TEST(String, SHA512) {
  String correctoutput("811aa0c53c0039b6ead0ca878b096eed1d39ed873fd2d2d270abfb9ca620d3ed561c565d6dbd1114c323d38e3f59c00df475451fc9b30074f2abda3529df2fa7");
  String s("Test string");
  String hash = s.SHA512();
  EXPECT_EQ(correctoutput, hash);
  EXPECT_EQ(128, hash.size());
  hash = s.SHA512(true);
  EXPECT_EQ(64, hash.size());
  unsigned char rawhash[] = { 0x81, 0x1a, 0xa0, 0xc5, 0x3c, 0x00, 0x39, 0xb6, 0xea, 0xd0, 0xca, 0x87, 0x8b, 0x09, 0x6e, 0xed, 0x1d, 0x39, 0xed, 0x87, 0x3f
    , 0xd2, 0xd2, 0xd2, 0x70, 0xab, 0xfb, 0x9c, 0xa6, 0x20, 0xd3, 0xed, 0x56, 0x1c, 0x56, 0x5d, 0x6d, 0xbd, 0x11, 0x14, 0xc3, 0x23, 0xd3, 0x8e, 0x3f, 0x59
    , 0xc0, 0x0d, 0xf4, 0x75, 0x45, 0x1f, 0xc9, 0xb3, 0x00, 0x74, 0xf2, 0xab, 0xda, 0x35, 0x29, 0xdf, 0x2f, 0xa7 };
  EXPECT_EQ(String((const char*) rawhash, sizeof(rawhash)), hash);
};

};
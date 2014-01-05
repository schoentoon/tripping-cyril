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

#include "String.h"

using namespace trippingcyril;

TEST(String, Bool) {
  String t(true);
  EXPECT_EQ(t, "true");
  EXPECT_TRUE(t.ToBool());
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
};

TEST(String, Int) {
  String leet((int) 1337);
  EXPECT_EQ(leet, "1337");
  String ninethousand((unsigned int) 9001);
  EXPECT_EQ(ninethousand, "9001");
  EXPECT_EQ(1337, leet.ToUInt());
  EXPECT_EQ(9001, ninethousand.ToInt());
};

TEST(String, Long) {
  String leet((long) 1337);
  EXPECT_EQ(leet, "1337");
  String ninethousand((unsigned long) 9001);
  EXPECT_EQ(ninethousand, "9001");
  EXPECT_EQ(1337, leet.ToULong());
  EXPECT_EQ(9001, ninethousand.ToLong());
};

TEST(String, LongLong) {
  String leet((long long) 1337);
  EXPECT_EQ(leet, "1337");
  String ninethousand((unsigned long long) 9001);
  EXPECT_EQ(ninethousand, "9001");
  EXPECT_EQ(1337, leet.ToULongLong());
  EXPECT_EQ(9001, ninethousand.ToLongLong());
};

TEST(String, Float) {
  String pi((float) 3.14159265359, 3);
  EXPECT_EQ(pi, "3.142");
  EXPECT_FLOAT_EQ(3.142, pi.ToFloat());
};

TEST(String, Double) {
  String pi((double) 3.14159265359, 5);
  EXPECT_EQ(pi, "3.14159");
  EXPECT_DOUBLE_EQ(3.14159, pi.ToDouble());
};
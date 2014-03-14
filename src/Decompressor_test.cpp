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

#include "Compressor.h"
#include "Decompressor.h"

using namespace trippingcyril;

namespace tests {

static const String IPSUM = "Bacon ipsum dolor sit amet jowl drumstick chuck, shankle "
  "chicken meatball filet mignon pork belly doner turducken meatloaf capicola. "
  "Frankfurter meatloaf jerky ham filet mignon shankle turducken beef. Flank beef "
  "filet mignon, ham ground round sirloin short ribs tongue spare ribs landjaeger "
  "turkey pork loin. Chuck cow short ribs swine, salami meatball doner beef spare "
  "ribs turducken. Tenderloin jowl filet mignon meatloaf frankfurter doner.";

#ifndef _NO_GZIP

TEST(Decompressor, GZip) {
  StringWriter* writer = new StringWriter;
  GZipCompressor compressor(writer);
  EXPECT_LT(0, compressor.WriteString(IPSUM));
  EXPECT_LT(writer->GetBuffer().size(), compressor.totalBytesIn());
  GZipDecompressor decompressor;
  EXPECT_EQ(decompressor.WriteString(writer->GetBuffer()), IPSUM.size());
  ASSERT_EQ(compressor.totalBytesIn(), decompressor.size());
  for (int i = 0; i < decompressor.size(); ++i) {
    EXPECT_EQ(IPSUM.data()[i], decompressor.data()[i]);
  };
};

#endif //_NO_GZIP

};
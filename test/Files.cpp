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

#include "Files.h"

using namespace trippingcyril;

namespace test {

class Files : public ::testing::Test {
protected:
  virtual void SetUp() {
    File proc("/proc");
    if (proc.Exists() == false) {
      FAIL() << "/proc isn't mounted?? You may want to disable these tests";
    };
    File dev("/dev");
    if (dev.Exists() == false) {
      FAIL() << "/dev isn't mounted?? You may want to disable these tests";
    };
  };
};

TEST_F(Files, ProcVersion) {
  File version("/proc/version");
  EXPECT_TRUE(version.Exists());
  EXPECT_FALSE(version.IsOpen());
  EXPECT_FALSE(version.Delete());
  EXPECT_EQ(File::REGULAR, version.GetType());
  EXPECT_EQ(version.GetShortName(), "version");
};

TEST_F(Files, ProcCpuInfo) {
  File cpuinfo("/proc/cpuinfo");
  EXPECT_TRUE(cpuinfo.Exists());
  EXPECT_EQ(cpuinfo.GetShortName(), "cpuinfo");
  EXPECT_EQ(cpuinfo.GetSize(), 0); // This is /proc so everything seems empty ;)

  String buffer;
  EXPECT_FALSE(cpuinfo.IsOpen());
  EXPECT_FALSE(cpuinfo.ReadLine(buffer));
  EXPECT_FALSE(buffer.size() > 0);
  EXPECT_TRUE(cpuinfo.Open());
  ASSERT_TRUE(cpuinfo.IsOpen());
  EXPECT_TRUE(cpuinfo.ReadLine(buffer));
  EXPECT_TRUE(buffer.size() > 0);

  EXPECT_EQ(cpuinfo.GetUID(), 0); // Root owns /proc
  EXPECT_EQ(cpuinfo.GetGID(), 0);
};

TEST_F(Files, DevNull) {
  File null("/dev/null");
  EXPECT_EQ(null.GetShortName(), "null");
  EXPECT_EQ(File::CHARACTER, null.GetType());
};

TEST_F(Files, DirByWildcard) {
  File* tty = NULL;
  {
    Dir dev("/dev", "tty*");
    ASSERT_GT(dev.size(), 0); // Purely an assumption..
    tty = dev[0];
  }
  EXPECT_DEATH(delete tty, ""); // Testing the dev deconstructor
  Dir proc("/proc");
  EXPECT_GT(proc.size(), 0);
};

TEST_F(Files, TempFile) {
  String filename;
  {
    TempFile tmp;
    EXPECT_TRUE(tmp.IsOpen());
    EXPECT_EQ(tmp.GetSize(), 0);
    EXPECT_EQ(tmp.GetType(), File::TEMPORARY);
    filename = tmp.GetName();
  }
  File tmp(filename);
  EXPECT_FALSE(tmp.Exists());
};

};
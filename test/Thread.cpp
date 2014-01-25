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

#include "Thread.h"

using namespace trippingcyril;

namespace tests {

class JoinThread : public Thread {
public:
  JoinThread(const String& name, void* output)
  : Thread(name) {
    this->output = output;
  };
  void* run() {
    return output;
  };
private:
  void* output;
};

TEST(Thread, Join) {
  int* number = (int*) malloc(sizeof(int));
  *number = 1337;
  JoinThread* thread = new JoinThread("jointhread", number);
  EXPECT_EQ(0, thread->Self());
  EXPECT_FALSE(thread->Join());
  EXPECT_TRUE(thread->Start());
  EXPECT_NE(0, thread->Self());
  EXPECT_TRUE(thread->Join());
  int* newnumber = (int*) thread->getReturnedValue();
  EXPECT_EQ(number, newnumber);
  delete thread;
  free(number);
};

};
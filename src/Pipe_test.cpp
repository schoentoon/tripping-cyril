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

#include <deque>

#include "LibEventHelper.h"

#include "Pipe.h"
#include "Thread.h"

namespace test {

class Pipe : public LibEventTest {
};

class TestPipe : public trippingcyril::Pipe {
public:
  TestPipe(const Module* module)
  : trippingcyril::Pipe(module) {
    done = NULL;
  };
  virtual ~TestPipe() {
    if (expectedReads.empty() == false) {
      ADD_FAILURE() << "There were expected reads which I haven't seen.";
    };
  };
  virtual void OnRead(PipeIO* pipe) {
    char buf[4096];
    int ret = pipe->Read(buf, sizeof(buf));
    if (ret > 0) {
      if (expectedReads.empty()) {
        ADD_FAILURE() << "I wasn't expecting any read operations";
      } else {
        String expected = expectedReads.front();
        String incoming(buf, ret);
        EXPECT_EQ(expected, incoming);
        expectedReads.pop_front();
      };
    } else {
      FAIL() << "Our read operation in pipe failed! pipe->Read() returned " << ret;
    };
    if (done != NULL)
      *done = true;
    event_base_loopbreak(GetModule()->GetEventBase());
  };
  bool* done;
  deque<String> expectedReads;
};

TEST_F(Pipe, OnRead) {
  TestPipe* pipe = new TestPipe(event_base);
  bool done = false;
  pipe->done = &done;
  String testdata = "This is a simple test.";
  pipe->expectedReads.push_back(testdata);
  EXPECT_EQ(pipe->GetWriteIO()->Write(testdata), testdata.length());
  while (done == false && event_base->Loop());
  delete pipe;
};

class WriteThread : public Thread {
public:
  WriteThread(PipeIO* writePipe, const String& writeData)
  : Thread("pipewritethread") {
    this->writePipe = writePipe;
    this->writeData = writeData;
  };
  void* run() {
    EXPECT_EQ(writePipe->Write(writeData), writeData.length());
    return NULL;
  };
private:
  PipeIO* writePipe;
  String writeData;
};

TEST_F(Pipe, WriteFromThread) {
  TestPipe* pipe = new TestPipe(event_base);
  bool done = false;
  pipe->done = &done;
  String testdata = "This is a simple test.";
  pipe->expectedReads.push_back(testdata);
  WriteThread* thread = new WriteThread(pipe->GetWriteIO(), testdata);
  EXPECT_TRUE(thread->Start());
  while (done == false && event_base->Loop());
  EXPECT_TRUE(thread->Join());
  delete thread;
  delete pipe;
};

};
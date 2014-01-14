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

#include "StackTrace.h"

#include "Files.h"
#include "TermUtils.h"

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

namespace trippingcyril {
  namespace crash {

static bool interactive = false;
static bool coredump = false;

void SetInteractive(bool b) {
  interactive = b;
};
void SetCoreDump(bool b) {
  coredump = b;
};

static void StackTrace() {
  char pid_buf[32];
  snprintf(pid_buf, sizeof(pid_buf), "%d", getpid());
  char name_buf[512];
  name_buf[readlink("/proc/self/exe", name_buf, sizeof(name_buf)-1)] = '\0';
  int child_pid = fork();
  if (child_pid == 0) {
    if (interactive == true)
      execlp("gdb", "gdb", name_buf, pid_buf, NULL);
    else {
      dup2(2, 1);
      Dir::MakeDir("crashes");
      String crashFile("crashes/" + String(time(NULL)) + ".stack");
      if (freopen(crashFile.c_str(), "w+", stdout) == NULL)
        TermUtils::PrintStatus(false, "Couldn't freopen " + crashFile + " :(");
      if (coredump)
        execlp("gdb", "gdb", "--batch", "-f", "-n", "-ex", "gcore", "-ex", "thread apply all bt full", name_buf, pid_buf, NULL);
      else
        execlp("gdb", "gdb", "--batch", "-f", "-n", "-ex", "thread apply all bt full", name_buf, pid_buf, NULL);
    };
    TermUtils::PrintStatus(false, "You shouldn't see this message, if you do something is very very wrong!");
  } else
    waitpid(child_pid, NULL, 0);
};

static void CrashHandler(int sig, siginfo_t* info, void* f) {
  StackTrace();
  kill(getpid(), sig);
  abort();
  /* Not reached */
};

bool InitCrashHandler() {
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND | SA_SIGINFO;
  act.sa_sigaction = CrashHandler;
  if (sigaction(SIGABRT, &act, NULL) != 0)
    return false;
  if (sigaction(SIGBUS, &act, NULL) != 0)
    return false;
  if (sigaction(SIGFPE, &act, NULL) != 0)
    return false;
  if (sigaction(SIGILL, &act, NULL) != 0)
    return false;
  if (sigaction(SIGSEGV, &act, NULL) != 0)
    return false;
  return true;
};

  };
};
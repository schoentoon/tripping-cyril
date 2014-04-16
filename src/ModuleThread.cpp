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

#include "ModuleThread.h"

#include "StackTrace.h"
#include "Pipe.h"
#include "TermUtils.h"
#include "Global.h"

namespace trippingcyril {

class OnCrashPipe : public Pipe {
public:
  OnCrashPipe()
  : Pipe() {
  };
  virtual ~OnCrashPipe() {
  };
  struct OnCrashStruct {
    Module* module;
    bool reload : 1;
  };
protected:
  virtual void OnRead() {
    OnCrashStruct data;
    while (Read((char*) &data, sizeof(data)) == sizeof(data) && data.module != NULL) {
      const String path = data.module->GetPath();
      String sRetMsg;
      bool success = Global::Get()->UnloadModule(data.module->GetModName(), sRetMsg);
      TermUtils::PrintStatus(success, sRetMsg);
      if (data.reload)
        Global::Get()->LoadModule(path);
    };
  };
};

static OnCrashPipe* onCrashPipe = new OnCrashPipe;

ModuleThread::ModuleThread(Module* pModule)
: Thread(pModule->GetModName(), pModule)
, unloadOnCrash(pModule->unloadOnCrash)
, reloadOnCrash(pModule->reloadOnCrash) {
  module = pModule;
  module->modThread = this;
};

ModuleThread::~ModuleThread() {
};

void* ModuleThread::run() {
  module->event_base = event_base_new();
  module->OnLoaded();
  if (unloadOnCrash == false || setjmp(jmp_buffer) == 0) {
    if (unloadOnCrash) {
      struct sigaction act;
      bzero(&act, sizeof(act));
      sigfillset(&act.sa_mask);
      act.sa_flags = SA_RESETHAND|SA_SIGINFO;
      act.sa_sigaction = ModuleThread::CrashHandler;
      sigaction(SIGABRT, &act, NULL);
      sigaction(SIGBUS,  &act, NULL);
      sigaction(SIGFPE,  &act, NULL);
      #ifdef SIGILL
      sigaction(SIGILL,  &act, NULL);
      #endif
      sigaction(SIGSEGV, &act, NULL);
    }
    while (shouldContinue())
      event_base_loop(module->event_base, EVLOOP_ONCE);
  };
  return NULL;
};

void ModuleThread::CrashHandler(int sig, siginfo_t* info, void* f) {
  Thread* thread = thread::ThreadManager::Get()->getCurrentThread();
  if (thread != NULL) {
    crash::StackTrace();
    ModuleThread* modThread = (ModuleThread*) thread;
    if (modThread) {
      OnCrashPipe::OnCrashStruct data;
      data.module = modThread->module;
      data.reload = modThread->reloadOnCrash;
      onCrashPipe->Write((const char*) &data, sizeof(data));
      longjmp(modThread->jmp_buffer, 1);
    } else
      perror("What the hell man..");
  } else
    perror("What the hell man..");
};

};
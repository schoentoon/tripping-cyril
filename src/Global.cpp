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

#include "Global.h"

#include <dlfcn.h>
#include <signal.h>
#include <setjmp.h>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "Pipe.h"
#include "Thread.h"
#include "Files.h"
#include "Module.h"
#include "StackTrace.h"
#include "TermUtils.h"

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

Global::Global() {
  SSL_library_init();
  ERR_load_CRYPTO_strings();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
  RAND_poll();
  event_base = event_base_new();
  dns_base = evdns_base_new(event_base, 1);
};

Global::~Global() {
  event_base_free(event_base);
  evdns_base_free(dns_base, 0);
};

void Global::Loop() {
  while (true)
    event_base_dispatch(event_base);
};

class ModuleThread : public Thread {
public:
  ModuleThread(Module* pModule)
  : Thread(pModule->GetModName())
  , unloadOnCrash(pModule->unloadOnCrash)
  , reloadOnCrash(pModule->reloadOnCrash) {
    module = pModule;
    module->modThread = this;
  };
  virtual ~ModuleThread() {
  };
protected:
  void* run() {
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
private:
  static void CrashHandler(int sig, siginfo_t* info, void* f) {
    Thread* thread = ThreadManager::Get()->getCurrentThread();
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
  jmp_buf jmp_buffer;
  Module* module;
  const bool unloadOnCrash;
  const bool reloadOnCrash;
};

bool Global::LoadModule(const String& path) {
  TermUtils::StatusPrinter status("Loading [" + path + "]");
  File f(path);
  for (unsigned int i = 0; i < modules.size(); i++) {
    if (modules[i]->GetModName() == f.GetShortName()) {
      status.PrintStatus(false, "Module [" + f.GetShortName() + "] is already loaded");
      return false;
    };
  };
  String modname = f.GetShortName();
  String::size_type sPos = modname.rfind('.');
  if (sPos != String::npos)
    modname = modname.substr(0, sPos);
  String retMsg;
  Module* module = Module::LoadModule(path, modname, retMsg);
  if (module == NULL) {
    status.PrintStatus(false, retMsg);
    return false;
  };
  status.PrintStatus(true, retMsg);
  modules.push_back(module);
  if (module->wantsThread) {
    ModuleThread* thread = new ModuleThread(module);
    thread->Start();
  } else
    module->OnLoaded();
  return true;
};

bool Global::UnloadModule(const String& modName, String& retMsg) {
  vector<Module*>::iterator iter;
  for (iter = modules.begin(); iter != modules.end(); ++iter) {
    if ((*iter)->GetModName() == modName)
      break;
  };
  if (iter == modules.end()) {
    retMsg = "Module [" + modName + "] isn't loaded?";
    return false;
  };
  ModHandle so = (*iter)->so;
  if (so != NULL) {
    delete *iter;
    modules.erase(iter);
    if (dlclose(so) == 0) {
      retMsg = "Module [" + modName + "] unloaded";
      return true;
    } else {
      retMsg = "Unable to unload module [" + modName + "] [" + dlerror() + "]";
      return false;
    };
  };
  retMsg = "Unable to unload module [" + modName + "]";
  return false;
};

Module* Global::FindModule(const String& module) {
  vector<Module*>::const_iterator iter;
  for (iter = modules.begin(); iter != modules.end(); ++iter) {
    if ((*iter)->GetModName() == module)
      return *iter;
  };
  return NULL;
};

InterModuleData Global::ModuleInternalApiCall(const String& module, int method, void* arg) {
  Module* mod = FindModule(module);
  if (mod == NULL)
    return InterModuleData();
  void* data = mod->InternalApiCall(method, arg);
  return InterModuleData(data, mod, method);
};

};
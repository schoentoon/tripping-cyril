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

#ifndef _MODULETHREAD_H
#define _MODULETHREAD_H

#include <setjmp.h>
#include <signal.h>
#include <strings.h>

#include "Thread.h"
#include "Module.h"

namespace trippingcyril {

class ModuleThread : public thread::Thread {
public:
  ModuleThread(Module* pModule);
  virtual ~ModuleThread();
protected:
  void* run();
private:
  static void CrashHandler(int sig, siginfo_t* info, void* f);
  jmp_buf jmp_buffer;
  Module* module;
  const bool unloadOnCrash;
  const bool reloadOnCrash;
};

};

#endif //_MODULETHREAD_H
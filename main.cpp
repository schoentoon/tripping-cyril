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
#include "TermUtils.h"
#include "Global.h"
#include "Files.h"

#include <iostream>
#include <cstring>

using namespace trippingcyril;

class ModuleLoader : public FileObserverCallback {
public:
  ModuleLoader() {
    mask = IN_CLOSE_WRITE;
    Dir::MakeDir("./sys");
    File null("/dev/null");
    null.Copy("./sys/loadmodule", true);
    null.Copy("./sys/unloadmodule", true);
  };
protected:
  enum Mode {
    UNLOAD,
    LOAD,
    UNKNOWN
  };
  virtual void OnCloseWrite(File& file) {
    if (file.GetSize() == 0)
      return; // Prevent a recursive loop..
    Mode mode = UNKNOWN;
    if (std::strcmp(file.GetShortName().c_str(), "unloadmodule") == 0)
      mode = UNLOAD;
    else if (std::strcmp(file.GetShortName().c_str(), "loadmodule") == 0)
      mode = LOAD;
    if (file.Open()) {
      String line;
      while (file.ReadLine(line)) {
        line.Trim();
        String msg;
        bool ret = false;
        switch (mode) {
        case UNLOAD:
          ret = Global::Get()->UnloadModule(line, msg);
          break;
        case LOAD:
          ret = Global::Get()->LoadModule(line, msg);
          break;
        case UNKNOWN:
        default:
          TermUtils::PrintStatus(false, "Hey there buddy, what are you trying to do?..");
        };
        TermUtils::PrintStatus(ret, msg);
      };
    };
    File null("/dev/null");
    null.Copy(file.GetName(), true);
  };
};

int main(int argc, char **argv) {
  {
    if (crash::InitCrashHandler() == true)
      TermUtils::PrintStatus(true, "Registered our crash handler");
    else
      TermUtils::PrintStatus(false, "Something went wrong while registering our crash handler");
    String msg;
    bool ret = Global::Get()->LoadModule("modules/sample.so", msg);
    TermUtils::PrintStatus(ret, msg);
    FileObserver::Get()->Register("./sys", new ModuleLoader);
  }
  Global::Get()->Loop();
  return 0;
};

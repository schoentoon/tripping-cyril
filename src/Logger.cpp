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

#include "Logger.h"

namespace trippingcyril {

FileLogger::FileLogger(File& pFile)
: file(pFile) {
}

void FileLogger::Log(const String& msg) {
  File log(file); // This way it'll automatically close itself ^_^
  if (log.Open(O_WRONLY | O_APPEND | O_CREAT))
    log.WriteString(msg);
}

SysLogger::SysLogger(int pPriority)
: priority(pPriority) {
}

void SysLogger::Log(const String& msg) {
  syslog(priority, "%s", msg.c_str());
}


};
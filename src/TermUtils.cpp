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

#include "TermUtils.h"

#include "Files.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

namespace trippingcyril {

void TermUtils::PrintStatus(bool status, const String& pMsg) {
  String msg(pMsg);
  msg.Trim();
  if (status)
    fprintf(stdout, " \033[1m\033[34m[\033[32m ok \033[34m]\033[39m\033[22m %s\n", msg.c_str());
  else
    fprintf(stdout, " \033[1m\033[34m[\033[31m !! \033[34m]\033[39m\033[22m %s\n", msg.c_str());
  fflush(stdout);
};

void TermUtils::PrintError(const String& msg) {
  PrintStatus(false, msg);
};

bool TermUtils::WritePidFile(const String& path) {
  StatusPrinter status("Writing pid to " + path);
  File file(path);
  if (file.Open(O_WRONLY | O_CREAT)) {
    String data(getpid());
    if (file.WriteString(data) != (int) data.size()) {
      status.PrintStatus(false, "Failed to write to " + path + " (" + strerror(errno) + ")");
      return false;
    };
    status.PrintStatus(true, "Succesfully wrote pid to " + path);
    return true;
  } else {
    status.PrintStatus(false, "Failed to open " + path + " (" + strerror(errno) + ")");
    return false;
  };
};

void TermUtils::PrintMotd() {
  const String motd =
  "  _______   _             _\n"
  " |__   __| (_)           (_)\n"
  "    | |_ __ _ _ __  _ __  _ _ __   __ _\n"
  "    | | '__| | '_ \\| '_ \\| | '_ \\ / _` |\n"
  "    | | |  | | |_) | |_) | | | | | (_| |\n"
  "    |_|_|  |_| .__/| .__/|_|_| |_|\\__, |\n"
  "      _____  | |   | |_ _          __/ |\n"
  "     / ____| |_|   |_(_) |        |___/\n"
  "    | |    _   _ _ __ _| |\n"
  "    | |   | | | | '__| | |\n"
  "    | |___| |_| | |  | | |\n"
  "     \\_____\\__, |_|  |_|_|\n"
  "            __/ |\n"
  "           |___/\n";
  fprintf(stdout, "%s", motd.c_str());
  fflush(stdout);
};

int TermUtils::GetCPUCount() {
  return sysconf(_SC_NPROCESSORS_ONLN);
};

TermUtils::StatusPrinter::StatusPrinter(const String& start_message) {
  called = false;
  String msg(start_message);
  msg.Trim();
  fprintf(stdout, " \033[1m\033[34m[\033[0m .. \033[1m\033[34m]\033[39m\033[22m %s", msg.c_str());
  fflush(stdout);
};

TermUtils::StatusPrinter::~StatusPrinter() {
  if (called == false)
    PrintStatus(false, "You never called PrintStatus");
};

void TermUtils::StatusPrinter::PrintStatus(bool status, const String& msg) {
  called = true;
  fprintf(stdout, "\r");
  fflush(stdout);
  TermUtils::PrintStatus(status, msg);
};

};
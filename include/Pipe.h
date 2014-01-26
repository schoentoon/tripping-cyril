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

#ifndef _PIPE_H
#define _PIPE_H

#include <event2/event.h>

#include "Module.h"
#include "Files.h"

namespace trippingcyril {

class PipeIO;

class Pipe {
public:
  Pipe(const Module* pModule);
  virtual ~Pipe();
  PipeIO* GetReadIO() { return readPipe; };
  PipeIO* GetWriteIO() { return writePipe; };
  virtual void OnRead(PipeIO* pipe) {};
  const Module* GetModule() const { return module; };
private:
  const Module* module;
  PipeIO* readPipe;
  PipeIO* writePipe;
  struct event* read_event;
  static void EventCallback(evutil_socket_t fd, short event, void* arg);
};

class PipeIO : public File {
public:
  virtual bool Exists() const { return false; };
  virtual FileType GetType() const { return UNKNOWN; };
  virtual off_t GetSize() const { return -1; };
  virtual time_t GetATime() const { return time(NULL); };
  virtual time_t GetMTime() const { return time(NULL); };
  virtual time_t GetCTime() const { return time(NULL); };
  virtual uid_t GetUID() const { return -1; };
  virtual gid_t GetGID() const { return -1; };
  virtual bool Delete() { return false; };
  virtual bool Move(const String& newpath, bool overwrite = false) { return false; };
  virtual bool Copy(const String& newpath, bool overwrite = false) { return false; };
  virtual bool Chmod(mode_t mode) { return false; };
  virtual bool Seek(off_t pos) { return false; };
  virtual bool Truncate(off_t len = 0) { return false; };
  virtual bool Sync() { return false; };
  virtual void Close() {};
  virtual bool ReadFile(String& data, size_t maxSize = 512 * 1024) { return false; };
private:
  PipeIO(int pFd)
  : File("pipe") {
    fd = pFd;
  };
  virtual ~PipeIO() {};
  friend class Pipe;
};

};

#endif //_PIPE_H
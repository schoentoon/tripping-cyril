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

/**
 * @brief A wrapper class for pipes
 */
class Pipe {
public:
  /**
   * General constructor
   * @param module The module to register this pipe on
   */
  Pipe(const Module* pModule);
  /**
   * Deconstructor, will close both ends of the pipe
   */
  virtual ~Pipe();
  /** @return The reader for this pipe */
  PipeIO* GetReadIO() const { return readPipe; };
  /** @return The writer for this pipe */
  PipeIO* GetWriteIO() const { return writePipe; };
  /**
   * Override this method with your own stuff to do when read operations happen
   */
  virtual void OnRead(PipeIO* pipe) {};
  /** @return The module we registered with */
  const Module* GetModule() const { return module; };
private:
  // @cond
  const Module* module;
  PipeIO* readPipe;
  PipeIO* writePipe;
  struct event* read_event;
  static void EventCallback(evutil_socket_t fd, short event, void* arg);
  // @endcond
};

/**
 * @brief Wrapper for the file descriptors from the pipe, same interface as File
 */
class PipeIO : public File {
public:
  /** @brief Disabled, we are not a file. */
  virtual bool Exists() const { return false; };
  virtual FileType GetType() const { return UNKNOWN; };
  /** @brief Disabled, we are not a file. */
  virtual off_t GetSize() const { return -1; };
  /** @brief Disabled, we are not a file. */
  virtual time_t GetATime() const { return time(NULL); };
  /** @brief Disabled, we are not a file. */
  virtual time_t GetMTime() const { return time(NULL); };
  /** @brief Disabled, we are not a file. */
  virtual time_t GetCTime() const { return time(NULL); };
  /** @brief Disabled, we are not a file. */
  virtual uid_t GetUID() const { return -1; };
  /** @brief Disabled, we are not a file. */
  virtual gid_t GetGID() const { return -1; };
  /** @brief Disabled, we are not a file. */
  virtual bool Delete() { return false; };
  /** @brief Disabled, we are not a file. */
  virtual bool Move(const String& newpath, bool overwrite = false) { return false; };
  /** @brief Disabled, we are not a file. */
  virtual bool Copy(const String& newpath, bool overwrite = false) { return false; };
  /** @brief Disabled, we are not a file. */
  virtual bool Chmod(mode_t mode) { return false; };
  /** @brief Disabled, we are not a file. */
  virtual bool Seek(off_t pos) { return false; };
  /** @brief Disabled, we are not a file. */
  virtual bool Truncate(off_t len = 0) { return false; };
  /** @brief Disabled, we are not a file. */
  virtual bool Sync() { return false; };
  /**
   * @brief Disabled, we are not a file.
   * Even though this is a reading operation it is disabled anyway due to the way
   * the internals of it work. It'll either read till maxSize or till an error or
   * EOF, which will probably all never happen.
   */
  virtual bool ReadFile(String& data, size_t maxSize = 512 * 1024) { return false; };
private:
  // @cond
  PipeIO(int pFd)
  : File("pipe") {
    fd = pFd;
  };
  virtual ~PipeIO() {};
  friend class Pipe;
  // @endcond
};

};

#endif //_PIPE_H
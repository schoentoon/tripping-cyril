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

#ifndef _FILES_H
#define _FILES_H

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <vector>

#include "String.h"

namespace trippingcyril {

class File {
public:
  File(const String& path);
  virtual ~File();

  enum FileType {
    REGULAR,
    DIRECTORY,
    CHARACTER,
    BLOCK,
    FIFO,
    LINK,
    SOCK,
    UNKNOWN
  };

  bool Exists() const;
  FileType GetType() const; // UNKNOWN may also be an error
  off_t GetSize() const;
  time_t GetATime() const;
  time_t GetMTime() const;
  time_t GetCTime() const;
  uid_t GetUID() const;
  gid_t GetGID() const;
  bool IsOpen() const;
  String GetName() const;

  bool Delete();
  bool Move(const String& newpath, bool overwrite = false);
  bool Copy(const String& newpath, bool overwrite = false);
  bool Chmod(mode_t mode);
  bool Seek(off_t pos);
  bool Sync();
  bool Open(int flags = O_RDONLY, mode_t mode = 0644);
  int Read(char* buffer, size_t len);
  bool ReadLine(String& data, const String& delimiter = "\n");
  bool ReadFile(String& data, size_t maxSize = 512 * 1024);
  int Write(const char* buffer, size_t len);
  int Write(const String& data);
  void Close();
  bool HadError() const { return hadError; };
  void ResetError() { hadError = false; };
private:
  String buffer;
  String filename;
  int fd;
  bool hadError;
};

};

#endif //_FILES_H
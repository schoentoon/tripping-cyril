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

#include "Files.h"

#include <stdio.h>

namespace trippingcyril {

File::File(const String& path) {
  fd = -1;
  hadError = false;
  filename = path;
};

File::~File() {
  Close();
};

bool File::Exists() const {
  struct stat st;
  return (stat(filename.c_str(), &st) == 0);
};

File::FileType File::GetType() const {
  struct stat st;
  if (stat(filename.c_str(), &st) != 0)
    return UNKNOWN;
  if (S_ISREG(st.st_mode))
    return REGULAR;
  else if (S_ISDIR(st.st_mode))
    return DIRECTORY;
  else if (S_ISCHR(st.st_mode))
    return CHARACTER;
  else if (S_ISBLK(st.st_mode))
    return BLOCK;
  else if (S_ISFIFO(st.st_mode))
    return FIFO;
  else if (S_ISLNK(st.st_mode))
    return LINK;
  else if (S_ISSOCK(st.st_mode))
    return SOCK;
  return UNKNOWN;
};

off_t File::GetSize() const {
  struct stat st;
  if (stat(filename.c_str(), &st) != 0)
    return 0;
  return (S_ISREG(st.st_mode)) ? st.st_size : 0;
};

time_t File::GetATime() const {
  struct stat st;
  return (stat(filename.c_str(), &st) != 0) ? 0 : st.st_atime;
};

time_t File::GetMTime() const {
  struct stat st;
  return (stat(filename.c_str(), &st) != 0) ? 0 : st.st_mtime;
};

time_t File::GetCTime() const {
  struct stat st;
  return (stat(filename.c_str(), &st) != 0) ? 0 : st.st_ctime;
};

uid_t File::GetUID() const {
  struct stat st;
  return (stat(filename.c_str(), &st) != 0) ? 0 : st.st_uid;
};

gid_t File::GetGID() const {
  struct stat st;
  return (stat(filename.c_str(), &st) != 0) ? 0 : st.st_gid;
};

bool File::IsOpen() const {
  return (fd != -1);
};

String File::GetName() const {
  return filename;
};

bool File::Delete() {
  return (unlink(filename.c_str()) == 0);
};

bool File::Move(const String& newpath, bool overwrite) {
  if (overwrite == false) {
    File newfile(newpath);
    if (newfile.Exists())
      return false;
  };
  return (rename(filename.c_str(), newpath.c_str()) == 0);
};

#define WRITE_BUFFER 8192

bool File::Copy(const String& newpath, bool overwrite) {
  File newfile(newpath);
  if (overwrite == false) {
    if (newfile.Exists())
      return false;
  };
  if (Open() == false)
    return false;
  if (newfile.Open(O_WRONLY | O_CREAT | O_TRUNC) == false)
    return false;
  char buf[WRITE_BUFFER];
  int len = 0;
  while ((len = Read(buf, sizeof(buf)))) {
    if (len < 0) {
      Close(); // Our copy is only a partial copy at this point, get rid of it.
      newfile.Close();
      newfile.Delete();
      return false;
    };
    newfile.Write(buf, len);
  };
  Close();
  newfile.Close();
  return true;
};

bool File::Chmod(mode_t mode) {
  return (chmod(filename.c_str(), mode) == 0);
};

bool File::Seek(off_t pos) {
  if (fd != -1 && lseek(fd, pos, SEEK_SET) == pos)
    return true;
  hadError = true;
  return false;
};

bool File::Sync() {
  if (fd != -1 && fsync(fd) == 0)
    return true;
  hadError = true;
  return false;
};

bool File::Open(int flags, mode_t mode) {
  mode |= O_NOCTTY;
  fd = open(filename.c_str(), flags, mode);
  if (fd < 0) {
    hadError = true;
    return false;
  };
  return true;
};

int File::Read(char* buffer, size_t len) {
  if (fd == -1)
    return -1;
  int res = read(fd, buffer, len);
  if (res < 0)
    hadError = true;
  return res;
};

#define READ_BUF 8192

bool File::ReadLine(String& data, const String& delimiter) {
  if (fd == -1)
    return false;
  char buf[READ_BUF];
  int len;
  do {
    String::size_type find = buffer.find(delimiter);
    if (find != String::npos) {
      data = buffer.substr(0, find + delimiter.length());
      buffer.erase(0, find + delimiter.length());
      return true;
    };
    len = Read(buf, sizeof(buf));
    if (len > 0)
      buffer.append(buf, len);
  } while (len > 0);
  // We are at the end of the file, or some error happend
  if (buffer.empty() == false) {
    // But here, have the rest of the file
    data = buffer;
    buffer.clear();
    return true;
  };
  return false;
};

bool File::ReadFile(String& data, size_t maxSize) {
  if (fd == -1)
    return false;
  char buf[READ_BUF];
  size_t bytesRead = 0;
  data.clear();
  while (bytesRead < maxSize) {
    int len = Read(buf, sizeof(buf));
    if (len < 0) // Error
      return false;
    if (len == 0) // EOF
      return true;
    data.append(buf, len);
    bytesRead += len;
  };
  return false; // maxSize reached
};

int File::Write(const char* buffer, size_t len) {
  if (fd == -1)
    return -1;
  int res = write(fd, buffer, len);
  if (res < 0)
    hadError = true;
  return res;
};

int File::Write(const String& data) {
  return Write(data.data(), data.size());
};

void File::Close() {
  if (fd > 0) {
    if (close(fd) < 0)
      hadError = true;
    fd = -1;
  };
};

};
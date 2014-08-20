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
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "Global.h"
#include "TermUtils.h"

namespace trippingcyril {

File::File(const String& path)
: _filename(path)
, _fd(-1) {
  String::size_type sPos = path.rfind('/');
  if (sPos != String::npos)
    _shortname = path.substr(sPos + 1);
  else
    _shortname = path;
};

File::~File() {
  Close();
};

bool File::Exists() const {
  struct stat st;
  return (stat(_filename.c_str(), &st) == 0);
};

File::FileType File::GetType() const {
  struct stat st;
  if (stat(_filename.c_str(), &st) != 0)
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
  if (stat(_filename.c_str(), &st) != 0)
    return 0;
  return (S_ISREG(st.st_mode)) ? st.st_size : 0;
};

time_t File::GetATime() const {
  struct stat st;
  return (stat(_filename.c_str(), &st) != 0) ? 0 : st.st_atime;
};

time_t File::GetMTime() const {
  struct stat st;
  return (stat(_filename.c_str(), &st) != 0) ? 0 : st.st_mtime;
};

time_t File::GetCTime() const {
  struct stat st;
  return (stat(_filename.c_str(), &st) != 0) ? 0 : st.st_ctime;
};

uid_t File::GetUID() const {
  struct stat st;
  return (stat(_filename.c_str(), &st) != 0) ? 0 : st.st_uid;
};

gid_t File::GetGID() const {
  struct stat st;
  return (stat(_filename.c_str(), &st) != 0) ? 0 : st.st_gid;
};

bool File::Delete() {
  return (unlink(_filename.c_str()) == 0);
};

bool File::Move(const String& newpath, bool overwrite) {
  if (overwrite == false) {
    File newfile(newpath);
    if (newfile.Exists())
      return false;
  };
  return (rename(_filename.c_str(), newpath.c_str()) == 0);
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
  return (chmod(_filename.c_str(), mode) == 0);
};

bool File::Seek(off_t pos, int whence) {
  if (_fd != -1 && lseek(_fd, pos, whence) >= 0)
    return true;
  return false;
};

off_t File::Tell() {
  if (_fd == -1) return 0;
  return lseek(_fd, 0, SEEK_CUR);
};

bool File::Truncate(off_t len) {
  if (_fd != -1 && ftruncate(_fd, len) == 0) {
    _buffer.clear();
    return true;
  };
  return false;
};

bool File::Sync() {
  if (_fd != -1 && fsync(_fd) == 0)
    return true;
  return false;
};

bool File::Open(int flags, mode_t mode) {
  mode |= O_NOCTTY;
  _fd = open(_filename.c_str(), flags, mode);
  if (_fd < 0)
    return false;
  return true;
};

int File::Read(char* buffer, size_t len) {
  if (_fd == -1)
    return -1;
  return read(_fd, buffer, len);
};

#define READ_BUF 8192

bool File::ReadLine(String& data, const String& delimiter) {
  if (_fd == -1)
    return false;
  char buf[READ_BUF];
  int len;
  do {
    String::size_type find = _buffer.find(delimiter);
    if (find != String::npos) {
      data = _buffer.substr(0, find + delimiter.length());
      _buffer.erase(0, find + delimiter.length());
      return true;
    };
    len = Read(buf, sizeof(buf));
    if (len > 0)
      _buffer.append(buf, len);
  } while (len > 0);
  // We are at the end of the file, or some error happend
  if (_buffer.empty() == false) {
    // But here, have the rest of the file
    data = _buffer;
    _buffer.clear();
    return true;
  };
  return false;
};

bool File::ReadFile(String& data, size_t maxSize) {
  if (_fd == -1)
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
  if (_fd == -1)
    return -1;
  char* p = (char*) buffer;
  int left = len;
  while (left > 0) {
    int wrote = write(_fd, p, std::min(BUFFER_SIZE, left));
    if (wrote < 0)
      return wrote;
    left -= wrote;
    p += wrote;
  };
  return len;
};

void File::Close() {
  if (_fd > 0) {
    close(_fd);
    _fd = -1;
  };
};

TempFile::TempFile()
: File("") {
  char tmp[] = "/tmp/trippingcyril.XXXXXX";
  _fd = mkstemp(tmp);
  _filename = tmp;
  String::size_type sPos = _filename.rfind('/');
  if (sPos != String::npos)
    _shortname = _filename.substr(sPos + 1);
  else
    _shortname = _filename;
};

TempFile::TempFile(const TempFile &that)
: File(that._filename)
{
};

TempFile::~TempFile() {
  Delete();
};

Dir::Dir(const String& dir, const String& wildcard) {
  DIR* d = opendir(dir.empty() ? "." : dir.c_str());
  if (d != NULL) {
    struct dirent* de;
    while ((de = readdir(d)) != NULL) {
      if ((strcmp(de->d_name, ".") == 0) || (strcmp(de->d_name, "..") == 0))
        continue;
      if (wildcard.empty() == false && (String(de->d_name).WildCmp(wildcard)) == false)
        continue;
      push_back(new File(dir + "/" + de->d_name));
    };
  };
};

Dir::~Dir() {
  while (empty() == false) {
    File* f = back();
    pop_back();
    delete f;
  };
};

String Dir::GetCWD() {
  char* dir = getcwd(NULL, 0);
  if (dir) {
    String ret(dir);
    free(dir);
    return ret;
  }
  return "";
};

bool Dir::MakeDir(const String& path, mode_t mode) {
  int ret = mkdir(path.c_str(), mode);
  return ret == 0;
};

FileObserver::FileObserver() {
  _inotifyfd = inotify_init();
  if (_inotifyfd == -1) {
    TermUtils::PrintError("An error occured in inotify_init(), exiting.");
    abort();
  };
  struct bufferevent* bev = bufferevent_socket_new(Global::Get()->GetEventBase(), _inotifyfd, 0);
  bufferevent_setcb(bev, FileObserver::readcb, NULL, NULL, this);
  bufferevent_enable(bev, EV_READ);
};

bool FileObserver::Register(const String& directory, FileObserverCallback* callback) {
  if (callback == NULL)
    return false;
  File dir(directory);
  if (dir.Exists() == false || dir.GetType() != File::DIRECTORY)
    return false;
  int wd = inotify_add_watch(_inotifyfd, directory.c_str(), callback->_mask);
  if (wd == -1)
    return false;
  String folder(directory);
  folder.TrimRight("/");
  _folders[wd] = folder;
  _callbacks[wd] = callback;
  return true;
};

bool FileObserver::Unregister(const String& directory) {
  for (map<int, String>::iterator iter = _folders.begin(); iter != _folders.end(); ++iter) {
    if (directory == iter->second) {
      int wd = iter->first;
      _folders.erase(iter);
      map<int, FileObserverCallback*>::iterator c_iter = _callbacks.find(wd);
      if (c_iter != _callbacks.end()) {
        if (c_iter->second != NULL)
          delete c_iter->second;
        _callbacks.erase(c_iter);
      };
      if (inotify_rm_watch(_inotifyfd, wd) == -1)
        return false;
      return true;
    };
  };
  return false;
};

#define BUF_LEN (1024 * (sizeof(struct inotify_event) + NAME_MAX + 1))

void FileObserver::readcb(struct bufferevent* bev, void* arg) {
  FileObserver* observer = (FileObserver*) arg;
  char buf[BUF_LEN];
  size_t numRead;
  while ((numRead = bufferevent_read(bev, buf, sizeof(buf)))) {
    char* p;
    for (p = buf; p < buf + numRead;) {
      struct inotify_event *event = (struct inotify_event*) p;
      if (event->len > 0) {
        String filename(event->name, event->len);
        map<int, String>::const_iterator path_iter = observer->_folders.find(event->wd);
        String path;
        if (path_iter != observer->_folders.end())
          path = path_iter->second;
        String filepath = path + "/" + filename;
        map<int, FileObserverCallback*>::iterator iter = observer->_callbacks.find(event->wd);
        if (iter != observer->_callbacks.end() && iter->second != NULL) {
          File file(filepath);
          if (event->mask & IN_ACCESS)
            iter->second->OnAccess(file);
          if (event->mask & IN_MODIFY)
            iter->second->OnModify(file);
          if (event->mask & IN_ATTRIB)
            iter->second->OnAtrributeChanged(file);
          if (event->mask & IN_CLOSE_WRITE)
            iter->second->OnCloseWrite(file);
          if (event->mask & IN_CLOSE_NOWRITE)
            iter->second->OnCloseNoWrite(file);
          if (event->mask & IN_DELETE)
            iter->second->OnDelete(file);
          if (event->mask & IN_CREATE)
            iter->second->OnCreate(file);
        };
      };
      p += sizeof(struct inotify_event) + event->len;
    };
  };
};

};
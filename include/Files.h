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

#include <event2/bufferevent.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/inotify.h>
#include <vector>
#include <map>

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
    TEMPORARY,
    UNKNOWN
  };

  bool Exists() const;
  virtual FileType GetType() const; // UNKNOWN may also be an error
  off_t GetSize() const;
  time_t GetATime() const;
  time_t GetMTime() const;
  time_t GetCTime() const;
  uid_t GetUID() const;
  gid_t GetGID() const;
  bool IsOpen() const;
  String GetName() const;
  String GetShortName() const;

  bool Delete();
  bool Move(const String& newpath, bool overwrite = false);
  bool Copy(const String& newpath, bool overwrite = false);
  bool Chmod(mode_t mode);
  bool Seek(off_t pos);
  bool Truncate(off_t len = 0);
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
protected:
  String buffer;
  String filename;
  String shortname;
  int fd;
  bool hadError;
};

class TempFile : public File {
public:
  TempFile();
  virtual ~TempFile();
  FileType GetType() const { return TEMPORARY; };
};

class Dir : public vector<File*> {
public:
  Dir(const String& dir, const String& wildcard = "");
  virtual ~Dir();
  static String GetCWD();
  static bool MakeDir(const String& path, mode_t mode = 0700);
};

class FileObserver;

class FileObserverCallback {
public:
  FileObserverCallback() {
    mask = 0; // Any way to set this automatically correctly based on the methods that are overriden?
  };
  virtual ~FileObserverCallback() {};
protected:
  virtual void OnAccess(File& file) {};
  virtual void OnModify(File& file) {};
  virtual void OnAtrributeChanged(File& file) {};
  virtual void OnCloseWrite(File& file) {};
  virtual void OnCloseNoWrite(File& file) {};
  virtual void OnDelete(File& file) {};
  virtual void OnCreate(File& file) {};
  int mask;
  friend class FileObserver;
};

class FileObserver {
public:
  static FileObserver* Get() {
    static FileObserver* singleton = new FileObserver();
    return singleton;
  };
  bool Register(const String& directory, FileObserverCallback* callback);
  bool Unregister(const String& directory);
  size_t amountWatchedFolders() { return folders.size(); };
private:
  FileObserver();
  map<int, String> folders;
  map<int, FileObserverCallback*> callbacks;
  int inotifyfd;
  static void readcb(struct bufferevent* bev, void* arg);
};

};

#endif //_FILES_H
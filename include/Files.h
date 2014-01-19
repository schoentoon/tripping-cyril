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

/**
 * @brief A general file access class
 */
class File {
public:
  File(const String& path);
  virtual ~File();

  enum FileType {
    REGULAR, ///< A regular file
    DIRECTORY, ///< A directory
    CHARACTER, ///< A character device
    BLOCK, ///< A block device
    FIFO, ///< A fifo see <a href="http://man7.org/linux/man-pages/man7/fifo.7.html">fifo(7)</a>
    LINK, ///< Either a symbolic or hard link
    SOCK, ///< A unix socket
    TEMPORARY, ///< Really just a regular file, but will be removed destruction
    UNKNOWN ///< Either not implemented yet or an error occured.
  };

  /** @return true if the file exists */
  bool Exists() const;
  virtual FileType GetType() const; // UNKNOWN may also be an error
  /** @return The size of the file, only works with REGULAR files, 0 otherwise */
  off_t GetSize() const;
  /** @return Last access time of this file. */
  time_t GetATime() const;
  /** @return Last modification time of this file. */
  time_t GetMTime() const;
  /** @return Creation time of this file. */
  time_t GetCTime() const;
  /** @return UID of this file. */
  uid_t GetUID() const;
  /** @return GID of this file. */
  gid_t GetGID() const;
  /** @return true if successfully opened with Open() @see Open */
  bool IsOpen() const;
  /** @return The path of this file, may be relative depends on what you passed to the constructor */
  String GetName() const;
  /** @return The filename without any path. */
  String GetShortName() const;

  /** @return true if successfully deleted. */
  bool Delete();
  /** @return true if successfully moved. */
  bool Move(const String& newpath, bool overwrite = false);
  /** @return true if successfully copied. */
  bool Copy(const String& newpath, bool overwrite = false);
  /** @return true if permissions were successfully modified. */
  bool Chmod(mode_t mode);
  /** @return true if successfully moved to this spot in the file, similar to 
   * <a href="http://man7.org/linux/man-pages/man2/lseek.2.html">lseek(2)</a> */
  bool Seek(off_t pos);
  /** @return true if successfully truncated, similar to
   * <a href="http://man7.org/linux/man-pages/man2/ftruncate.2.html">truncate(2)</a> */
  bool Truncate(off_t len = 0);
  /** @return true if successfully synchronized, similar to
   * <a href="http://man7.org/linux/man-pages/man2/fsync.2.html">fsync(2)</a> */
  bool Sync();
  /** @return true if successfully opened, similar to
   * <a href="http://man7.org/linux/man-pages/man2/open.2.html">open(2)</a> */
  bool Open(int flags = O_RDONLY, mode_t mode = 0644);
  /** @return The amount of bytes read into the buffer, similar to
   * <a href="http://man7.org/linux/man-pages/man2/read.2.html">read(2)</a>*/
  int Read(char* buffer, size_t len);
  /**
   * Reads up to the delimiter into data
   * @param data Reference to a string to read into, string is purely used as a buffer
   * @param delimiter Up until what character we should read
   * @return true if the delimiter was found
   */
  bool ReadLine(String& data, const String& delimiter = "\n");
  /**
   * Reads up until maxSize into data
   * @param data Reference to a string to read into, string is purely used as a buffer
   * @param maxSize The maximum amount of bytes to read into our buffer
   * @return true if the file was entirely read into our buffer
   */
  bool ReadFile(String& data, size_t maxSize = 512 * 1024);
  /** @return The amount of bytes written into the file, similar to
   * <a href="http://man7.org/linux/man-pages/man2/write.2.html">write(2)</a> */
  int Write(const char* buffer, size_t len);
  /** @see Write */
  int Write(const String& data);
  /** Closes the file if it was previously opened with Open */
  void Close();
protected:
  // @cond
  String buffer;
  String filename;
  String shortname;
  int fd;
  // @endcond
};

/**
 * @brief An easy way to get temporary files in /tmp which will be deleted upon
 * destruction of the object.
 */
class TempFile : public File {
public:
  /** Creates a temporary file in /tmp */
  TempFile();
  /** This deconstructor will remove the file */
  virtual ~TempFile();
  /** @return Will always return TEMPORARY */
  FileType GetType() const { return TEMPORARY; };
};

/**
 * @brief A general directory access class
 */
class Dir : public vector<File*> {
public:
  /**
   * @param dir The directory this object is about
   * @param wildcard The wildcard we should match on all the files, for example
   * <code>Dir moddir("./modules", "*.so");</code> would only put files ending
   * with .so in this vector. If it isn't a directory the vector will simply be
   * empty.
   */
  Dir(const String& dir, const String& wildcard = "");
  virtual ~Dir();
  /** @return The current working directory */
  static String GetCWD();
  /** @return true if path was successfully created */
  static bool MakeDir(const String& path, mode_t mode = 0700);
};

class FileObserver;

/**
 * @brief An inotify wrapper class callback
 * @see FileObserver
 */
class FileObserverCallback {
public:
  FileObserverCallback() {
    mask = 0; // Any way to set this automatically correctly based on the methods that are overriden?
  };
  virtual ~FileObserverCallback() {};
protected:
  /**
   * Requires IN_ACCESS, will get called once a file was accessed.
   */
  virtual void OnAccess(File& file) {};
  /**
   * Requires IN_MODIFY, will get called once a file was modified.
   */
  virtual void OnModify(File& file) {};
  /**
   * Requires IN_ATTRIB, will get called once the metadata of a file was modified.
   */
  virtual void OnAtrributeChanged(File& file) {};
  /**
   * Requires IN_CLOSE_WRITE, will get called once a file opened for writing is closed.
   */
  virtual void OnCloseWrite(File& file) {};
  /**
   * Requires IN_CLOSE_NOWRITE, will get called once a file opened not for writing is closed.
   */
  virtual void OnCloseNoWrite(File& file) {};
  /**
   * Requires IN_DELETE, will get called once a file/directory is deleted.
   */
  virtual void OnDelete(File& file) {};
  /**
   * Requires IN_CREATE, will get called once a new file/directory was created.
   */
  virtual void OnCreate(File& file) {};
  /**
   * The mask to register with, note that this will decide what On* methods will get called, see
   * <a href="http://man7.org/linux/man-pages/man7/inotify.7.html">inotify(7)</a> for help with the mask
   */
  int mask;
  friend class FileObserver;
};

/**
 * @brief An inotify wrapper class
 */
class FileObserver {
public:
  /**
   * FileObserver is currently a singleton, use this method to get it.
   */
  static FileObserver* Get() {
    static FileObserver* singleton = new FileObserver();
    return singleton;
  };
  /**
   * @brief Registers a FileObserverCallback for directory
   * @param directory The directory to watch
   * @param callback The FileObserverCallback to use for this directory, can't be NULL
   * @return true if registered successfully
   */
  bool Register(const String& directory, FileObserverCallback* callback);
  /**
   * @brief Unregisters a directory from being watched
   * @param directory The directory to stop watching
   * @return true if unregistered successfully
   */
  bool Unregister(const String& directory);
  /** @return The amount of folders being watched */
  size_t amountWatchedFolders() { return folders.size(); };
private:
  FileObserver();
  // @cond
  map<int, String> folders;
  map<int, FileObserverCallback*> callbacks;
  int inotifyfd;
  static void readcb(struct bufferevent* bev, void* arg);
  // @endcond
};

};

#endif //_FILES_H
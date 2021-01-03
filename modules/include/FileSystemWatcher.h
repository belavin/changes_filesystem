#ifndef PFW_FILESYSTEM_WATCHER_H
#define PFW_FILESYSTEM_WATCHER_H

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>

#include "modules/include/Filter.h"
#include "modules/include/NativeInterface.h"


class FileSystemWatcher : public NativeInterface
{
  public:
    FileSystemWatcher(const fs::path &path,
                      std::chrono::milliseconds    sleepDuration,
                      CallBackSignatur             callback);
    ~FileSystemWatcher();
};



#endif

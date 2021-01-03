#ifndef PFW_RUN_LOOP_H
#define PFW_RUN_LOOP_H

#include <filesystem>
#include <thread>
#include <atomic>

#include "modules/include/SingleshotSemaphore.h"




class FSEventsService;

class RunLoop
{
  public:
    RunLoop(FSEventsService *                eventsService,
            const std::filesystem::path &    path,
            const std::chrono::milliseconds &latency);

    bool isLooping();

    ~RunLoop();

  private:
    void work();

    FSEventsService *               mEventsService;
    std::atomic<bool>               mExited;
    std::filesystem::path           mPath;
    const std::chrono::milliseconds mLatency;
    std::thread                     mRunLoopThread;
    SingleshotSemaphore             mReadyForCleanup;
    std::atomic<bool>               mStarted;
    std::atomic<bool>               mShutdown;
};



#endif

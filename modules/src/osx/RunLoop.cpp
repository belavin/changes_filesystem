#include "modules/include/osx/RunLoop.h"

#include "modules/include/osx/FSEventsService.h"



RunLoop::RunLoop(FSEventsService *                eventsService,
                 const std::filesystem::path &    path,
                 const std::chrono::milliseconds &latency)
    : mEventsService(eventsService)
    , mExited(false)
    , mPath(path)
    , mLatency(latency)
    , mStarted(false)
{
    mRunLoopThread = std::thread([](RunLoop *rl) { rl->work(); }, this);
    mStarted       = mRunLoopThread.joinable();

    if (!mStarted) {
        mEventsService->sendError("Неожиданное завершение работы службы.");
    }
}

bool RunLoop::isLooping() { return mStarted && !mExited; }

RunLoop::~RunLoop()
{
    mShutdown = true;
    if (!mStarted) {
        return;
    }

    mReadyForCleanup.wait();
//    CFRunLoopStop(mRunLoop);

    mRunLoopThread.join();
}

void RunLoop::work()
{
}

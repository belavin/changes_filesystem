#include "modules/include//osx/FSEventsService.h"

#include "modules/include/osx/RunLoop.h"

#include <chrono>
#include <iostream>



FSEventsService::FSEventsService(std::shared_ptr<Filter>          filter,
                                 std::filesystem::path            path,
                                 const std::chrono::milliseconds &latency)
    : mFilter(filter)
{
    std::error_code ec;
    mPath = std::filesystem::canonical(path, ec);
    if (ec) {
        mFilter->sendError("Не удалось открыть каталог.");
        mRunLoop = NULL;
        return;
    }

    mRunLoop = new RunLoop(this, mPath, latency);

    if (!mRunLoop->isLooping()) {
        delete mRunLoop;
        mRunLoop = NULL;
        return;
    }
}

FSEventsService::~FSEventsService()
{
    if (mRunLoop != NULL) {
        delete mRunLoop;
    }
}



void FSEventsService::dispatch(std::vector<EventPtr> &&events)
{
    mFilter->filterAndNotify(std::move(events));
}

const std::filesystem::path &FSEventsService::rootPath() { return mPath; }

void FSEventsService::sendError(const std::string &errorMsg)
{
    mFilter->sendError(errorMsg);
}

bool FSEventsService::isWatching()
{
    return mRunLoop != NULL && mRunLoop->isLooping();
}

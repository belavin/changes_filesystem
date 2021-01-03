#ifndef PFW_FS_EVENTS_SERVICE_H
#define PFW_FS_EVENTS_SERVICE_H

#include <filesystem>
#include <map>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <time.h>
#include <vector>

#include "modules/include/Filter.h"


class RunLoop;

class FSEventsService
{
  public:
    FSEventsService(std::shared_ptr<Filter>          filter,
                    std::filesystem::path            path,
                    const std::chrono::milliseconds &latency);


    void                         sendError(const std::string &errorMsg);
    bool                         isWatching();
    const std::filesystem::path &rootPath();

    ~FSEventsService();

  private:
    void dispatch(std::vector<EventPtr> &&events);

    std::filesystem::path   mPath;
    RunLoop *               mRunLoop;
    std::shared_ptr<Filter> mFilter;
};



#endif

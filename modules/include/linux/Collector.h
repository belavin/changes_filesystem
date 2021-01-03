#ifndef PFW_COLLECTOR_H
#define PFW_COLLECTOR_H

#include <atomic>
#include <chrono>
#include <mutex>
#include <pthread.h>
#include <vector>

#include "modules/include/Filter.h"
#include "modules/include/SingleshotSemaphore.h"



class Collector
{
  public:
    Collector(std::shared_ptr<Filter>   filter,
              std::chrono::milliseconds sleepDuration);
    ~Collector();

    static void  finish(void *args);
    static void *work(void *args);

    void sendError(const std::string &errorMsg);
    void insert(std::vector<EventPtr> &&events);
    void push_back(EventType type, const std::filesystem::path &relativePath);

  private:
    void sendEvents();

    std::shared_ptr<Filter>   mFilter;
    std::chrono::milliseconds mSleepDuration;
    pthread_t                 mRunner;
    std::atomic<bool>         mStopped;
    std::vector<EventPtr>     inputVector;
    std::mutex                event_input_mutex;
};



#endif

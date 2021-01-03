#ifndef PFW_NATIVE_INTERFACE_H
#define PFW_NATIVE_INTERFACE_H


#include "modules/include/linux/InotifyService.h"
using NativeImplementation = InotifyService;


#include "modules/include/Filter.h"
#include <vector>


namespace fs = std::filesystem;

class NativeInterface
{
  public:
    NativeInterface(const fs::path &                path,
                    const std::chrono::milliseconds latency,
                    CallBackSignatur                callback);
    ~NativeInterface();

    bool isWatching();

  private:
    std::shared_ptr<Filter>               _filter;
    std::unique_ptr<NativeImplementation> _nativeInterface;
};


#endif

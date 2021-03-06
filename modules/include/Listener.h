#ifndef PFW_LISTENER_H
#define PFW_LISTENER_H

#include <algorithm>
#include <functional>
#include <map>
#include <mutex>
#include <utility>




template <template <class...> class Template, class T>
struct is_instantiation : std::false_type {
};
template <template <class...> class Template, class... Ts>
struct is_instantiation<Template, Template<Ts...>> : std::true_type {
};

template <class CallbackType>
class Listener
{
    static_assert(is_instantiation<std::function, CallbackType>::value,
                  "CallbackType должен быть экземпляром std::function");

  public:
    using CallbackHandle = int;

  private:
    std::map<CallbackHandle, CallbackType> mListeners;
    std::mutex                             mListenersMutex;
    int                                    mHandleCount{0};

  public:
    CallbackHandle registerCallback(const CallbackType callback)
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        mListeners[++mHandleCount] = callback;
        return mHandleCount;
    }

    void deregisterCallback(const CallbackHandle &id)
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        auto                        it = mListeners.find(id);

        if (it != mListeners.end()) {
            mListeners.erase(it);
        }
    }

  protected:
    template <typename... Args>
    void notify(Args &&... args)
    {
        std::lock_guard<std::mutex> lock(mListenersMutex);
        for (const auto &func : mListeners) {
            func.second(std::move(std::forward<Args>(args))...);
        }
    }
};



#endif

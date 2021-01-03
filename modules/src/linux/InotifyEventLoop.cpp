#include "modules/include/linux/InotifyEventLoop.h"

#include <sys/ioctl.h>

#include <csignal>
#include <iostream>

InotifyEventLoop::InotifyEventLoop(int             inotifyInstance,
                                   InotifyService *inotifyService)
    : mInotifyService(inotifyService)
    , mInotifyInstance(inotifyInstance)
    , mStopped(true)
{
    int result = pthread_create(&mEventLoop, nullptr, work, this);

    if (result != 0) {
        mInotifyService->sendError(
            "Не удалось запустить поток InotifyEventLoop. Код ошибки: " +
            std::string(strerror(errno)));
        return;
    }

    mThreadStartedSemaphore.wait();
}

bool InotifyEventLoop::isLooping() { return !mStopped; }

void InotifyEventLoop::created(inotify_event *event,
                               bool           isDirectoryEvent,
                               bool           sendInitEvents)
{
    if (event == NULL || mStopped) {
        return;
    }

    if (isDirectoryEvent) {
        mInotifyService->createDirectory(event->wd, strdup(event->name),
                                         sendInitEvents);
    } else {
        mInotifyService->create(event->wd, strdup(event->name));
    }
}
void InotifyEventLoop::modified(inotify_event *event)
{
    if (event == NULL || mStopped) {
        return;
    }

    mInotifyService->modify(event->wd, strdup(event->name));
}
void InotifyEventLoop::deleted(inotify_event *event, bool isDirectoryRemoval)
{
    if (event == NULL || mStopped) {
        return;
    }

    if (isDirectoryRemoval) {
        mInotifyService->removeDirectory(event->wd);
    } else {
        mInotifyService->remove(event->wd, strdup(event->name));
    }
}
void InotifyEventLoop::moveStart(inotify_event *     event,
                                 bool                isDirectoryEvent,
                                 InotifyRenameEvent &renameEvent)
{
    if (mStopped) {
        return;
    }

    renameEvent = InotifyRenameEvent(event, isDirectoryEvent);
}
void InotifyEventLoop::moveEnd(inotify_event *     event,
                               bool                isDirectoryEvent,
                               InotifyRenameEvent &renameEvent)
{
    if (mStopped) {
        return;
    }

    if (!renameEvent.isGood) {
        return created(event, isDirectoryEvent, false);
    }

    renameEvent.isGood = false;

    if (renameEvent.cookie != event->cookie) {
        if (renameEvent.isDirectory) {
            mInotifyService->removeDirectory(renameEvent.wd, renameEvent.name);
        }
        mInotifyService->remove(renameEvent.wd, renameEvent.name);

        return created(event, isDirectoryEvent, false);
    }

    if (renameEvent.isDirectory) {
        mInotifyService->moveDirectory(renameEvent.wd, renameEvent.name,
                                       event->wd, event->name);
    } else {
        mInotifyService->move(renameEvent.wd, renameEvent.name, event->wd,
                              event->name);
    }
}

void InotifyEventLoop::finish(void *args)
{
    InotifyEventLoop *eventLoop = reinterpret_cast<InotifyEventLoop *>(args);
    eventLoop->mStopped         = true;
}

void *InotifyEventLoop::work(void *args)
{
    pthread_cleanup_push(InotifyEventLoop::finish, args);

    static const int  BUFFER_SIZE = 16384;
    InotifyEventLoop *eventLoop   = reinterpret_cast<InotifyEventLoop *>(args);

    eventLoop->mStopped = false;
    eventLoop->mThreadStartedSemaphore.signal();

    InotifyRenameEvent renameEvent;

    while (!eventLoop->mStopped) {
        char buffer[BUFFER_SIZE];

        auto bytesRead =
            read(eventLoop->mInotifyInstance, &buffer, BUFFER_SIZE);

        if (eventLoop->mStopped) {
            break;
        } else if (bytesRead == 0) {
            eventLoop->mInotifyService->sendError(
                "Поток InotifyEventLoop mStopped, поскольку чтение вернуло 0.");
            break;
        } else if (bytesRead == -1) {
            // чтение было прервано
            if (errno == EINTR) {
                break;
            }
            eventLoop->mInotifyService->sendError(
                "Прочитать inotify не удается из-за ошибки: " +
                std::string(strerror(errno)));
            break;
        }

        ssize_t        position = 0;
        inotify_event *event    = nullptr;
        do {
            if (eventLoop->mStopped) {
                break;
            }
            event = (struct inotify_event *)(buffer + position);

            bool isDirectoryRemoval =
                event->mask & (uint32_t)(IN_IGNORED | IN_DELETE_SELF);
            bool isDirectoryEvent = event->mask & (uint32_t)(IN_ISDIR);

            if (renameEvent.isGood && event->cookie != renameEvent.cookie) {
                eventLoop->moveEnd(event, isDirectoryEvent, renameEvent);
            } else if (event->mask & (uint32_t)(IN_ATTRIB | IN_MODIFY)) {
                eventLoop->modified(event);
            } else if (event->mask & (uint32_t)IN_CREATE) {
                eventLoop->created(event, isDirectoryEvent);
            } else if (event->mask & (uint32_t)(IN_DELETE | IN_DELETE_SELF)) {
                eventLoop->deleted(event, isDirectoryRemoval);
            } else if (event->mask & (uint32_t)IN_MOVED_TO) {
                if (event->cookie == 0) {
                    eventLoop->created(event, isDirectoryEvent);
                    continue;
                }

                eventLoop->moveEnd(event, isDirectoryEvent, renameEvent);
            } else if (event->mask & (uint32_t)IN_MOVED_FROM) {
                if (event->cookie == 0) {
                    eventLoop->deleted(event, isDirectoryRemoval);
                    continue;
                }

                eventLoop->moveStart(event, isDirectoryEvent, renameEvent);
            } else if (event->mask & (uint32_t)IN_MOVE_SELF) {
                eventLoop->mInotifyService->remove(event->wd,
                                                   strdup(event->name));
                eventLoop->mInotifyService->removeDirectory(event->wd);
            }
        } while ((position += sizeof(struct inotify_event) + event->len) <
                 bytesRead);

        if (eventLoop->mStopped) {
            break;
        }

        size_t bytesAvailable = 0;
        if (ioctl(eventLoop->mInotifyInstance, FIONREAD, &bytesAvailable) < 0) {
            continue;
        }
        if (bytesAvailable == 0) {
            // Если у нас есть завершающее событие renameEvent, а в списке нет другого события
            // конвейер, тогда нам нужно завершить это renameEvent. В противном случае мы
            // потеряет информацию об ожидающем событии переименования.
            if (renameEvent.isGood) {
                if (renameEvent.isDirectory) {
                    eventLoop->mInotifyService->removeDirectory(
                        renameEvent.wd, renameEvent.name);
                }
                eventLoop->mInotifyService->remove(renameEvent.wd,
                                                   renameEvent.name);
            }
        }
    }

    pthread_cleanup_pop(1);
    return nullptr;
}

InotifyEventLoop::~InotifyEventLoop()
{
    if (mStopped) {
        return;
    }
    mStopped = true;

    // \ note (mathias): чтобы быть уверенным, что отмена потока не вызовет
    // прерывание всей программы, потому что другая библиотека реагирует на сигнал 32,
    // игнорируем сигнал extra. Дополнительно мы сохраняем предыдущие
    // обработчик для регистрации этого обработчика для повторного сигнала 32 в конце. (Это
    // только предположение, потому что мы наблюдали такое поведение (прерывание
    // вся программа при вызове dtor из NotifyEventLoop) в сочетании с
    // другие библиотеки без обнаружения первопричин.
    auto previousHandler = std::signal(32, SIG_IGN);

    auto errorCode = pthread_cancel(mEventLoop);
    if (errorCode != 0) {
        mInotifyService->sendError(
            "Не удалось отменить поток InotifyEventLoop. Код ошибки: " +
            std::to_string(errorCode));
        return;
    }

    errorCode = pthread_join(mEventLoop, NULL);
    if (errorCode != 0) {
        mInotifyService->sendError(
            "Не удалось присоединиться к потоку InotifyEventLoop. Код ошибки: " +
            std::to_string(errorCode));
    }

    if (previousHandler != SIG_ERR) {
        std::signal(32, previousHandler);
    }
}

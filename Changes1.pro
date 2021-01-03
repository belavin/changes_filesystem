TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
        modules/src/FileSystemWatcher.cpp \
        modules/src/Filter.cpp \
        modules/src/NativeInterface.cpp \
        modules/src/linux/Collector.cpp \
        modules/src/linux/InotifyEventLoop.cpp \
        modules/src/linux/InotifyNode.cpp \
        modules/src/linux/InotifyService.cpp \
        modules/src/linux/InotifyTree.cpp \
        modules/src/osx/FSEventsService.cpp \
        modules/src/osx/RunLoop.cpp

HEADERS += \
    modules/include/Event.h \
    modules/include/FileSystemWatcher.h \
    modules/include/Filter.h \
    modules/include/Listener.h \
    modules/include/NativeInterface.h \
    modules/include/SingleshotSemaphore.h \
    modules/include/linux/Collector.h \
    modules/include/linux/InotifyEventLoop.h \
    modules/include/linux/InotifyNode.h \
    modules/include/linux/InotifyService.h \
    modules/include/linux/InotifyTree.h \
    modules/include/osx/FSEventsService.h \
    modules/include/osx/RunLoop.h

LIBS += -L/usr/local/lib \
        -lpthread

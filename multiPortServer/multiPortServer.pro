QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#c++11
CONFIG += c++17


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    main.cpp \
    mainwindow.cpp \
    myserver.cpp



HEADERS += \
    mainwindow.h \
    myserver.h


FORMS += \
    mainwindow.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


unix|win32: LIBS += -L$$PWD/../vcpkg/installed/x86-windows/lib/ -lInfluxDB

INCLUDEPATH += $$PWD/../vcpkg/installed/x86-windows/include
DEPENDPATH += $$PWD/../vcpkg/installed/x86-windows/include

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../vcpkg/installed/x86-windows/lib/InfluxDB.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../vcpkg/installed/x86-windows/lib/libInfluxDB.a

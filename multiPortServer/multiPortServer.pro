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

#influxdb api
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../influxdb-cxx/build/lib/release/ -lInfluxDB
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../influxdb-cxx/build/lib/debug/ -lInfluxDB
else:unix: LIBS += -L$$PWD/../../influxdb-cxx/build/lib/ -lInfluxDB

INCLUDEPATH += $$PWD/../../influxdb-cxx/build
DEPENDPATH += $$PWD/../../influxdb-cxx/build

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../release/ -lcppkafka
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../debug/ -lcppkafka
else:unix: LIBS += -L$$PWD/../../ -lcppkafka

INCLUDEPATH += $$PWD/../../
DEPENDPATH += $$PWD/../../

QT -= gui
QT += sql

CONFIG += c++17
CONFIG -= app_bundle

linux-g++ | linux-g++-64 | linux-g++-32 {
  LIBS+=-lssl -lcrypto -I /usr/local/pgsql/include -L /usr/local/pgsql/lib -lpq
  CONFIG += -openssl-linked
  PKGCONFIG+=openssl
}

linux-g++ | linux-g++-64 | linux-g++-32 {
    include(qtservice/src/qtservice.pri)
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        Objects/logger.cpp \
        Objects/pgworker.cpp \
        Objects/task.cpp \
        main.cpp \
        service.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



HEADERS += \
    Objects/logger.h \
    Objects/pgworker.h \
    Objects/task.h \
    service.h

DISTFILES += \
    depends \
    qtservice/config.pri

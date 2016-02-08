#-------------------------------------------------
#
# Project created by QtCreator 2016-02-07T15:24:43
#
#-------------------------------------------------

QT       -= core gui

TARGET = Raknet
TEMPLATE = lib

DEFINES += RAKNET_LIBRARY

SOURCES += raknet.cpp

HEADERS += raknet.h\
        raknet_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

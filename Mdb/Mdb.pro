#-------------------------------------------------
#
# Project created by QtCreator 2016-02-07T21:31:31
#
#-------------------------------------------------

QT       -= core gui

TARGET = Mdb
TEMPLATE = lib
CONFIG += staticlib

SOURCES += mdb.c

HEADERS += lmdb.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

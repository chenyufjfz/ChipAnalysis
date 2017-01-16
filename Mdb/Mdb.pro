#-------------------------------------------------
#
# Project created by QtCreator 2016-02-07T21:31:31
#
#-------------------------------------------------

QT       -= core gui

TARGET = Mdb
TEMPLATE = lib
CONFIG += staticlib


SOURCES += mdb.c \
    midl.c

HEADERS += lmdb.h \
    midl.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
win32{
DEFINES += _CRT_SECURE_NO_WARNINGS \
    _CRT_NONSTDC_NO_DEPRECATE
}


CONFIG(debug, debug|release) {
DESTDIR = $$_PRO_FILE_PWD_/../lib/debug
} else {
DESTDIR = $$_PRO_FILE_PWD_/../lib/release
}

